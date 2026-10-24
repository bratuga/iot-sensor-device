#include "Config.h"
#include "HeatingCore.h"
#include "HeatingRuntime.h"
#include "HeatingSnapshot.h"
#include "HeatingReadPlan.h"
#include "Bme280Direct.h"
#include "WebUi.h"
// Optional private build settings. include/secrets.h is excluded by .gitignore.
// Remove it and rebuild to stop applying test overrides.
#if __has_include("secrets.h")
#include "secrets.h"
#endif
#ifdef LOCAL_AP_PASSWORD
static_assert(sizeof(LOCAL_AP_PASSWORD) == 1 ||
              (sizeof(LOCAL_AP_PASSWORD) >= 9 && sizeof(LOCAL_AP_PASSWORD) <= 64),
              "AP password must be empty or contain 8-63 characters");
#endif
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <DallasTemperature.h>
#include <ESPmDNS.h>
#include <OneWire.h>
#include <Update.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <knx/TunnelClient.h>
#include <memory>

namespace {
constexpr uint16_t LocalPort = 41000;
constexpr uint32_t SessionCooldown = 130000;
WebServer web(80);
WiFiUDP udp;
Bme280Direct bme;
TwoWire oledWire(1);
TwoWire *displayBus = &Wire;
String bmeI2cScan, oledI2cScan;
std::unique_ptr<OneWire> wire;
std::unique_ptr<DallasTemperature> ds;
std::unique_ptr<Adafruit_SSD1306> oled;
bool bmeOk = false, oledOk = false, converting = false, apActive = false, udpActive = false,
     mdns = false;
bool linkWasUp = false, needCooldown = false, updating = false, updateOk = false,
     uploadAllowed = false;
uint32_t lastUploadAt = 0, lostAt = 0, lastWifiAttempt = 0, lastSample = 0, conversionAt = 0,
         lastDisplay = 0, bootAt = 0, restartAt = 0;
bool restarting = false, firstSample = true;
IPAddress boundIp;
bool serialOverflow = false;
String csrf, lastEvent = "Arranque", apName, serialLine;
String detectedRom[8];
heating::RuntimeLayout heatingLayout;
constexpr unsigned MaxRooms=16;
unsigned roomCount=0;
String heatingZones[MaxRooms];
heating::Runtime heat;
struct HeatConfig {
 bool enabled=false;
 uint16_t command[9]={}, feedback[9]={}, enable[MaxRooms]={}, status[MaxRooms]={}, temperature[MaxRooms]={}, setpoint[MaxRooms]={};
 bool external[MaxRooms]={}, bmeFallback[MaxRooms]={};
 float margin[MaxRooms]={};
 uint32_t openMs=120000, maxAgeMs=300000;
} hc;
heating::ZoneInput hz[MaxRooms];
heating::ZoneInput effective[MaxRooms];
bool roomStatusDirty[MaxRooms]={}, roomReadCommand[MaxRooms]={}, roomReadStatus[MaxRooms]={};
bool setpointReceived[MaxRooms]={};
heating::Snapshot heatCache[MaxRooms];
bool heatCacheDirty=false, heatCacheUrgent=false, heatCacheSaveOk=true;
uint32_t heatCacheSavedAt=0;
void loadHeatingValues();
void saveHeatingValues(bool force=false);
bool zoneEnabled[MaxRooms]={}, zoneDemand[MaxRooms]={};
int zoneMode[MaxRooms]={}; // 0 automatic, 1 off, 2 manual on (30 minute lease)
uint32_t zoneManualAt[MaxRooms]={}, heatPoll=0, heatSend=0;
heating::ReadSweep heatReadSweep;
bool heatFeedbackRead[9]={};
uint32_t heatToken=1000, heatPending[9]={};
bool heatingSafe();
void heatingTelegram(const knxip::Telegram &t);
void heatingLoop(uint32_t now);
unsigned romCount = 0;
String logs[12];
unsigned logCount = 0, logNext = 0;
struct Reading {
  float value = NAN;
  bool valid = false;
  uint32_t at = 0;
  sensor::Publication publication;
};
Reading readings[5];
device::RelayState relays[4];
void setRelay(unsigned i, bool on);
uint32_t ipNumber(IPAddress ip) {
  return (uint32_t(ip[0]) << 24) | (uint32_t(ip[1]) << 16) | (uint32_t(ip[2]) << 8) | ip[3];
}
IPAddress ipAddress(uint32_t n) {
  return IPAddress(uint8_t(n >> 24), uint8_t(n >> 16), uint8_t(n >> 8), uint8_t(n));
}
String groupText(uint16_t g) {
  return String(g >> 11) + "/" + String((g >> 8) & 7) + "/" + String(g & 255);
}
String individualText(uint16_t a) {
  return String(a >> 12) + "." + String((a >> 8) & 15) + "." + String(a & 255);
}
void logTelegram(const String &s) {
  logs[logNext] = s;
  logNext = (logNext + 1) % 12;
  if (logCount < 12)
    ++logCount;
  Serial.println(s);
}
bool fresh(unsigned i, uint32_t now) {
  return readings[i].valid && !sensor::elapsed(now, readings[i].at, config.sampleMs * 3 + 1000);
}
class UdpTransport : public knxip::Transport {
  bool send(const knxip::Endpoint &p, const uint8_t *bytes, size_t size) override {
    if (!udpActive || WiFi.status() != WL_CONNECTED)
      return false;
    if (!udp.beginPacket(ipAddress(p.ip), p.port))
      return false;
    size_t n = udp.write(bytes, size);
    bool sent = udp.endPacket() == 1;
    return n == size && sent;
  }
} transport;
class Events : public knxip::Listener {
  void onTelegram(const knxip::Telegram &) override;
  void onEvent(knxip::Event e, uint16_t detail) override;
  void onSent(uint32_t token, bool ok) override {
    for(unsigned i=0;i<9;++i)if(token&&token==heatPending[i]){if(!heat.active(i)||!heat.output[i].sent)continue;heat.acknowledge(i,ok);heatFeedbackRead[i]=ok&&!heat.confirmed(i,heat.output[i].command,millis());if(!ok){heat.output[i].sent=false;heat.trip(millis(),heating::Runtime::Failure::Transport,int(i));}}
    if (token >= 1 && token <= 5)
      readings[token - 1].publication.complete(ok, millis());
    if (token >= 100 && token < 104) relays[token - 100].complete(ok);
  }
} events;
knxip::TunnelClient tunnel(transport, events);
const char *stateName() {
  switch (tunnel.state()) {
  case knxip::State::Stopped:
    return "parado";
  case knxip::State::Connecting:
    return "a ligar";
  case knxip::State::Connected:
    return "ligado";
  case knxip::State::Closing:
    return "a desligar";
  case knxip::State::Backoff:
    return "a aguardar reconexao";
  case knxip::State::Blocked:
    return "bloqueado";
  }
  return "desconhecido";
}
void Events::onEvent(knxip::Event e, uint16_t detail) {
  lastEvent = "Evento KNX " + String(int(e)) + " / " + String(detail);
  if (e == knxip::Event::Connected) {
    for(auto &pending:heatFeedbackRead)pending=false;
    heatReadSweep.start(roomCount);heatPoll=millis()-500;
    for (auto &r : relays) r.reconnect();
    lastEvent = "KNX ligado: " + individualText(tunnel.individualAddress());
    for (auto &r : readings)
      r.publication.reconnect();
  }
  Serial.println(lastEvent);
}
void Events::onTelegram(const knxip::Telegram &t) {
  String s = String(millis() / 1000) + "s " + individualText(t.source) + " > " +
             groupText(t.group) + " " +
             (t.operation == knxip::Operation::Read    ? "READ"
              : t.operation == knxip::Operation::Write ? "WRITE"
                                                       : "RESPONSE");
  for (unsigned i = 0; i < t.size; ++i) {
    char h[4];
    snprintf(h, sizeof(h), " %02X", t.data[i]);
    s += h;
  }
  logTelegram(s);
  heatingTelegram(t);
  if (updating || restarting) return;
  for (unsigned i = 0; i < 4; ++i) {
    const auto &r = config.relay[i];
    if (!r.enabled) continue;
    uint16_t command, feedback;
    sensor::parseGroup(r.commandGroup.c_str(), command);
    sensor::parseGroup(r.feedbackGroup.c_str(), feedback);
    bool value;
    if (t.operation == knxip::Operation::Write && t.group == command && t.asBool(value))
      setRelay(i, value);
    if (t.operation == knxip::Operation::Read && t.group == feedback)
      relays[i].requestRead();
  }
  if (t.operation != knxip::Operation::Read) return;
  for (unsigned i = 0; i < 5; ++i) {
    uint16_t group;
    uint8_t bytes[2];
    if (config.channel[i].enabled && sensor::parseGroup(config.channel[i].group.c_str(), group) &&
        group == t.group && fresh(i, millis()) && sensor::encodeDpt9(readings[i].value, bytes))
      tunnel.respond(group, bytes, 2, false, 0);
  }
}
void setRelay(unsigned i, bool on) {
  if (i >= 4 || !config.relay[i].enabled) return;
  digitalWrite(config.relay[i].gpio, device::relayLevel(on, config.relay[i].activeLow) ? HIGH : LOW);
  relays[i].command(on);
}
void initRelays() {
  for (unsigned i = 0; i < 4; ++i) {
    if (!config.relay[i].enabled) continue;
    // Preload OFF before enabling output; hardware pull resistors are still required at reset.
    digitalWrite(config.relay[i].gpio, device::relayLevel(false, config.relay[i].activeLow) ? HIGH : LOW);
    pinMode(config.relay[i].gpio, OUTPUT);
  }
}
void publishRelays(uint32_t now) {
  if (tunnel.state() != knxip::State::Connected) return;
  for (unsigned i = 0; i < 4; ++i) {
    auto &r = relays[i];
    if (!config.relay[i].enabled || !r.due(now)) continue;
    uint16_t group;
    if (!sensor::parseGroup(config.relay[i].feedbackGroup.c_str(), group)) continue;
    const uint8_t value = r.on ? 1 : 0;
    const bool response = r.readRequested;
    auto result = response ? tunnel.respond(group, &value, 1, true, 100 + i)
                           : tunnel.writeBool(group, r.on, 100 + i);
    if (result == knxip::Result::Queued) r.queued(now, response);
  }
}
void setReading(unsigned i, float value, bool valid, uint32_t now) {
  auto &r = readings[i];
  value += config.channel[i].offset;
  r.valid = valid && std::isfinite(value);
  if (i == 1)
    r.valid = r.valid && value >= 0 && value <= 100;
  r.value = r.valid ? value : NAN;
  r.at = now;
}
String scanI2c(TwoWire &bus) {
  String found;
  for (uint8_t address = 0x08; address <= 0x77; ++address) {
    bus.beginTransmission(address);
    if (bus.endTransmission() == 0) {
      char hex[7];
      snprintf(hex, sizeof(hex), "0x%02X", address);
      if (found.length()) found += ", ";
      found += hex;
    }
  }
  return found.length() ? found : "nenhum";
}
String bmeChipId(TwoWire &bus, bool repeatedStart) {
  uint8_t address = config.bmeAddress;
  bus.beginTransmission(address);
  bus.write(uint8_t(0xD0));
  uint8_t error = bus.endTransmission(!repeatedStart);
  if (error != 0) return "escrita falhou: " + String(error);
  if (bus.requestFrom(address, uint8_t(1)) != 1) return "leitura falhou";
  char result[7];
  snprintf(result, sizeof(result), "0x%02X", bus.read());
  return String(result);
}
void initSensors() {
  bool sharedOled = config.oledEnabled && config.oledSda < 0;
  if (config.bmeEnabled || sharedOled) {
    Wire.setPins(config.sda, config.scl);
    Wire.begin();
    Wire.setClock(50000);
    Wire.setTimeOut(50);
    delay(50);
    bmeI2cScan = scanI2c(Wire);
    delay(10);
    Serial.println("I2C principal: SDA=" + String(config.sda) + " SCL=" + String(config.scl) +
                   " encontrados=" + bmeI2cScan);
  }
  if (config.oledEnabled && !sharedOled) {
    oledWire.setPins(config.oledSda, config.oledScl);
    oledWire.begin();
    oledWire.setTimeOut(25);
    displayBus = &oledWire;
    oledI2cScan = scanI2c(oledWire);
    Serial.println("I2C OLED: SDA=" + String(config.oledSda) + " SCL=" + String(config.oledScl) +
                   " encontrados=" + oledI2cScan);
  } else {
    oledI2cScan = bmeI2cScan;
  }
  if (config.bmeEnabled) {
    bmeOk = bme.begin(config.bmeAddress, Wire);
    if (!bmeOk)
      Serial.println("BME falha: " + String(bme.lastError()) +
                     " reg=0x" + String(bme.lastRegister(), HEX) +
                     " | ID: " + bmeChipId(Wire, false) + " | scan: " + scanI2c(Wire));
    Serial.println(bmeOk ? "BME280: pronto" : "BME280: inicializacao falhou");
  }
  if (config.oneWire >= 0) {
    wire.reset(new OneWire(config.oneWire));
    ds.reset(new DallasTemperature(wire.get()));
    ds->begin();
    ds->setResolution(12);
    ds->setWaitForConversion(false);
    DeviceAddress address;
    for (unsigned i = 0; i < ds->getDeviceCount() && romCount < 8; ++i) {
      if (!ds->getAddress(address, i) || address[0] != 0x28)
        continue;
      char s[17];
      for (unsigned j = 0; j < 8; ++j)
        snprintf(s + 2 * j, 3, "%02X", address[j]);
      detectedRom[romCount++] = s;
    }
  }
  if (config.oledEnabled) {
    oled.reset(new Adafruit_SSD1306(128, config.oledHeight, displayBus, config.oledReset));
    oledOk = oled->begin(SSD1306_SWITCHCAPVCC, config.oledAddress, true, false);
    displayBus->beginTransmission(config.oledAddress);
    oledOk = (displayBus->endTransmission() == 0) && oledOk;
    if (oledOk) {
      oled->clearDisplay();
      oled->setTextSize(1);
      oled->setTextColor(SSD1306_WHITE);
      oled->display();
    }
  }
}
void sampleSensors(uint32_t now) {
  if (converting && sensor::elapsed(now, conversionAt, 800)) {
    converting = false;
    for (unsigned j = 0; j < 2; ++j) {
      uint8_t rom[8];
      float v = NAN;
      bool ok = sensor::parseRom(config.rom[j].c_str(), rom);
      if (ok)
        v = ds->getTempC(rom);
      setReading(3 + j, v, ok && v >= -55 && v <= 125, now);
    }
  }
  if (!firstSample && !sensor::elapsed(now, lastSample, config.sampleMs))
    return;
  firstSample = false;
  lastSample = now;
  if (config.bmeEnabled) {
    if (!bmeOk)
      bmeOk = bme.begin(config.bmeAddress, Wire);
    if (bmeOk) {
      float t = NAN, h = NAN, p = NAN;
      bool measured = bme.sample(t, h, p);
      bool ok = measured && std::isfinite(t) && std::isfinite(h) && std::isfinite(p) &&
                t >= -40 && t <= 85 &&
                h >= 0 && h <= 100 && p >= 30000 && p <= 110000;
      setReading(0, t, ok, now);
      setReading(1, h, ok, now);
      setReading(2, p, ok, now);
      if (!ok) {
        Serial.println("BME amostra falhou: " + String(bme.lastError()) +
                       " reg=0x" + String(bme.lastRegister(), HEX) +
                       " T=" + String(t) + " H=" + String(h) + " P=" + String(p));
        bmeOk = false;
      }
    }
  }
  if (ds && !converting) {
    ds->requestTemperatures();
    conversionAt = now;
    converting = true;
  }
}
void publish(uint32_t now) {
  if (tunnel.state() != knxip::State::Connected)
    return;
  for (unsigned i = 0; i < 5; ++i) {
    auto &c = config.channel[i];
    auto &r = readings[i];
    if (!c.enabled || !fresh(i, now) || !r.publication.due(r.value, now, c.policy))
      continue;
    uint16_t g;
    uint8_t bytes[2];
    if (sensor::parseGroup(c.group.c_str(), g) && sensor::encodeDpt9(r.value, bytes) &&
        tunnel.write(g, bytes, 2, false, i + 1) == knxip::Result::Queued)
      r.publication.queued(r.value, now);
  }
}
const char *apPassword() {
#ifdef LOCAL_AP_PASSWORD
  return LOCAL_AP_PASSWORD;
#else
  return config.adminPassword.c_str();
#endif
}
void openAp() {
  if (apActive)
    return;
  WiFi.mode(WIFI_AP_STA);
  apName = "Sensor-" + WiFi.macAddress();
  apName.replace(":", "");
  apActive = WiFi.softAP(apName.c_str(), apPassword());
  Serial.println("AP: " + apName + " | http://192.168.4.1 | utilizador admin");
}
void network(uint32_t now) {
  bool connected = WiFi.status() == WL_CONNECTED;
  if (udpActive && (!connected || WiFi.localIP() != boundIp)) {
    tunnel.networkLost(now);
    tunnel.stop(now);
    udp.stop();
    udpActive = false;
    needCooldown = true;
    lostAt = now;
  }
  if (!connected) {
    if (linkWasUp) {
      Serial.println("Wi-Fi perdido");
      if (mdns) {
        MDNS.end();
        mdns = false;
      }
    }
    linkWasUp = false;
    if (!config.ssid.isEmpty() && sensor::elapsed(now, lastWifiAttempt, 30000)) {
      lastWifiAttempt = now;
      WiFi.begin(config.ssid.c_str(), config.wifiPassword.c_str());
    }
    if (sensor::elapsed(now, bootAt, 60000) && !apActive)
      openAp();
    return;
  }
  if (!linkWasUp) {
    Serial.println("IP: " + WiFi.localIP().toString());
    linkWasUp = true;
  }
  if (!mdns) {
    mdns = MDNS.begin(config.name.c_str());
    if (mdns)
      MDNS.addService("http", "tcp", 80);
  }
  if (!config.knxEnabled || restarting || updating)
    return;
  if (udpActive && tunnel.state() == knxip::State::Stopped) {
    udp.stop();
    udpActive = false;
    needCooldown = true;
    lostAt = now;
  }
  if (!udpActive && (!needCooldown || sensor::elapsed(now, lostAt, SessionCooldown))) {
    boundIp = WiFi.localIP();
    if (!udp.begin(LocalPort))
      return;
    udpActive = true;
    needCooldown = false;
    knxip::Config k;
    IPAddress gateway;
    gateway.fromString(config.gateway);
    k.gateway = {ipNumber(gateway), config.port};
    k.maxTelegramsPerSecond = config.rate;
    if (!tunnel.start(k, {ipNumber(boundIp), LocalPort}, now)) {
      udp.stop();
      udpActive = false;
      lastEvent = "Falha ao iniciar KNX";
    }
  }
}
void pollTunnel(uint32_t now) {
  if (!udpActive || WiFi.status() != WL_CONNECTED)
    return;
  for (unsigned n = 0; n < 8; ++n) {
    int size = udp.parsePacket();
    if (!size)
      break;
    knxip::Endpoint peer = {ipNumber(udp.remoteIP()), udp.remotePort()};
    uint8_t buf[256];
    if (size > int(sizeof(buf))) {
      while (udp.available())
        udp.read(buf, sizeof(buf));
      continue;
    }
    int got = udp.read(buf, sizeof(buf));
    if (got == size)
      tunnel.onDatagram(peer, buf, got, now);
  }
  tunnel.poll(now);
}
void display(uint32_t now) {
  if (!oledOk || !sensor::elapsed(now, lastDisplay, 1000))
    return;
  lastDisplay = now;
  oled->clearDisplay();
  oled->setCursor(0, 0);
  oled->println(config.name);
  oled->println(stateName());
  static unsigned page = 0;
  page = (page + 1) % 8;
  if (page < 5) {
    unsigned i = page;
    oled->print(config.channel[i].name);
    oled->print(": ");
    if (fresh(i, now))
      oled->println(readings[i].value, i == 2 ? 0 : 1);
    else
      oled->println("--");
  } else if (logCount) {
    oled->println(logs[(logNext + 11) % 12]);
  } else
    oled->println(WiFi.localIP());
  oled->display();
}
void scheduleRestart() {
  if(!heatingSafe()){Serial.println("Para o aquecimento antes de reiniciar");return;}
  saveHeatingValues(true);
  tunnel.stop(millis());
  restarting = true;
  restartAt = millis();
}
void sendJson(JsonDocument &d, int code = 200) {
  String s;
  serializeJson(d, s);
  web.sendHeader("Cache-Control", "no-store");
  web.send(code, "application/json", s);
}
void error(int code, const String &msg) {
  DynamicJsonDocument d(512);
  d["error"] = msg;
  sendJson(d, code);
}
// Temporary local test mode. Set true when Web access protection is restored.
constexpr bool RequireWebAuthentication = false;
bool authenticated() {
  return !RequireWebAuthentication || web.authenticate("admin", apPassword());
}
bool auth() {
  if (authenticated())
    return true;
  web.requestAuthentication(DIGEST_AUTH, "Sensor KNX");
  return false;
}
bool mutation() {
  if (!auth())
    return false;
  if (web.header("X-CSRF-Token") != csrf) {
    error(403, "Sessao invalida; recarrega a pagina");
    return false;
  }
  return true;
}
bool heatingSafe() {
 if (!hc.enabled) return true;
 if (heat.armed || !heat.confirmed(8,false,millis())) return false;
 for (unsigned i=0;i<8;++i) if(heatingLayout.zone[i]>=0&&hc.command[i]&&!heat.confirmed(i,false,millis())) return false;
 return true;
}
bool parseHeatConfig(JsonDocument &d, HeatConfig &n) {
 if (!d.containsKey("control")) return true; // migrate mapping-only configuration
 JsonObject c=d["control"];
 if(!c["enabled"].is<bool>())return false;
 n.enabled=c["enabled"]; n.openMs=c["openMs"]|120000u; n.maxAgeMs=c["maxAgeMs"]|300000u;
 if(n.openMs<1000||n.openMs>600000||n.maxAgeMs<10000||n.maxAgeMs>3600000)return false;
 if(!c["actuators"].is<JsonArray>()||!c["rooms"].is<JsonArray>()||c["actuators"].size()!=9||c["rooms"].size()!=d["zones"].size())return false;
 uint16_t used[82]={}; unsigned count=0;
 auto group=[&](JsonVariant v,uint16_t &g){
   String text=v.as<String>();
   if(text.isEmpty()){g=0;return true;}
   if(!sensor::parseGroup(text.c_str(),g)||!g)return false;
   for(unsigned i=0;i<count;++i)if(used[i]==g)return false;
   for(const auto&r:config.relay){uint16_t x;if(r.enabled&&((sensor::parseGroup(r.commandGroup.c_str(),x)&&x==g)||(sensor::parseGroup(r.feedbackGroup.c_str(),x)&&x==g)))return false;}
   for(const auto&r:config.channel){uint16_t x;if(r.enabled&&sensor::parseGroup(r.group.c_str(),x)&&x==g)return false;}
   used[count++]=g;return true;
 };
 for(unsigned i=0;i<9;++i)if(!group(c["actuators"][i]["command"],n.command[i])||!group(c["actuators"][i]["feedback"],n.feedback[i]))return false;
 for(unsigned i=0;i<d["zones"].size();++i)if(!group(c["rooms"][i]["enable"],n.enable[i])||!group(c["rooms"][i]["status"],n.status[i])||!group(c["rooms"][i]["temperature"],n.temperature[i])||!group(c["rooms"][i]["setpoint"],n.setpoint[i]))return false;
 for(unsigned i=0;i<d["zones"].size();++i){
   JsonObject r=c["rooms"][i];
   if((r.containsKey("external")&&!r["external"].is<bool>())||(r.containsKey("bmeFallback")&&!r["bmeFallback"].is<bool>()))return false;
   n.external[i]=r["external"]|false;n.bmeFallback[i]=r["bmeFallback"]|false;
   n.margin[i]=r["margin"]|0.3f;
   if(!std::isfinite(n.margin[i])||n.margin[i]<0.1f||n.margin[i]>5)return false;
 }
 for(unsigned i=0;i<9;++i)if(bool(n.command[i])!=bool(n.feedback[i]))return false;
 if(n.enabled){
   if(!n.command[8]||!n.feedback[8])return false;
   for(unsigned i=0;i<8;++i)if(d["circuits"][i]["zone"].as<int>()>=0&&(!n.command[i]||!n.feedback[i]))return false;
   for(unsigned i=0;i<d["zones"].size();++i)if(!n.enable[i]||!n.status[i]||!n.temperature[i]||!n.setpoint[i])return false;
 }
 return true;
}
void loadHeatingValues() {
  for(unsigned i=0;i<MaxRooms;++i) heatCache[i]={};
  heatCacheDirty=heatCacheUrgent=false;
  Preferences p;
  if(!p.begin("heating",true))return;
  String value=p.isKey("values")?p.getString("values", ""):String();p.end();
  DynamicJsonDocument d(8192);
  if(deserializeJson(d,value)||d["version"]!=1||!d["rooms"].is<JsonArray>())return;
  for(unsigned i=0;i<roomCount;++i){
    auto &c=heatCache[i];c.temperatureGroup=hc.temperature[i];c.setpointGroup=hc.setpoint[i];
    for(JsonObject r:d["rooms"].as<JsonArray>()){
      if(hc.enable[i]&&r["enableGroup"]==hc.enable[i]&&r["enabled"].is<bool>())zoneEnabled[i]=r["enabled"];
      if(c.temperatureGroup&&r["temperatureGroup"]==c.temperatureGroup&&r["temperature"].is<float>()){
        float v=r["temperature"];if(std::isfinite(v)&&v>=-40&&v<=100){c.temperature=v;c.hasTemperature=true;}
      }
      if(c.setpointGroup&&r["setpointGroup"]==c.setpointGroup&&r["setpoint"].is<float>()){
        float v=r["setpoint"];if(std::isfinite(v)&&v>=5&&v<=35){c.setpoint=v;c.hasSetpoint=true;}
      }
    }
    c.restore(hz[i]);
  }
}
void saveHeatingValues(bool force) {
  if(!heatCacheDirty)return;
  uint32_t now=millis();
  if(!force&&!heatCacheUrgent&&heating::age(now,heatCacheSavedAt)<60000)return;
  // Failed writes are rate limited as well; an explicit restart can retry.
  if(!force&&!heatCacheSaveOk&&heating::age(now,heatCacheSavedAt)<60000)return;
  DynamicJsonDocument d(8192);d["version"]=1;
  JsonArray rooms=d.createNestedArray("rooms");
  for(unsigned i=0;i<roomCount;++i){
    auto r=rooms.createNestedObject();const auto &c=heatCache[i];
    r["temperatureGroup"]=hc.temperature[i];r["setpointGroup"]=hc.setpoint[i];r["enableGroup"]=hc.enable[i];r["enabled"]=zoneEnabled[i];
    if(c.hasTemperature)r["temperature"]=c.temperature;
    if(c.hasSetpoint)r["setpoint"]=c.setpoint;
  }
  String value;serializeJson(d,value);Preferences p;
  heatCacheSaveOk=p.begin("heating",false);
  if(heatCacheSaveOk){heatCacheSaveOk=p.putString("values",value)==value.length();p.end();}
  heatCacheSavedAt=now;
  if(heatCacheSaveOk){heatCacheDirty=false;heatCacheUrgent=false;}
  else Serial.println("Aquecimento: falha ao guardar ultimos valores KNX");
}
String heatingBlockReason() {
  if(!hc.enabled)return "Ativa a configuracao KNX do aquecimento e guarda.";
  if(tunnel.state()!=knxip::State::Connected)return "A aguardar ligacao KNX.";
  if(heat.fault){
    String detail;
    if(heat.faultOutput>=0){unsigned i=unsigned(heat.faultOutput);
      detail=(i==8?String("Bomba"):String("Circuito ")+String(i+1))+" (comando "+groupText(hc.command[i])+", feedback "+groupText(hc.feedback[i])+")";
    }
    using F=heating::Runtime::Failure;
    switch(heat.failure){
      case F::FeedbackTimeout:return "Falha: "+detail+" sem confirmacao em 30 s. Verifica o atuador e usa Reler valores KNX.";
      case F::Transport:return "Falha no envio KNX: "+detail+". Verifica a ligacao.";
      case F::LinkLost:return "Falha: ligacao KNX interrompida. Confirma as saidas OFF e reativa o controlo.";
      case F::PathLost:return "Falha: perdeu-se o feedback ON dos circuitos com a bomba comandada ON.";
      case F::UnexpectedPump:return "Falha: bomba ligada externamente; foi pedido OFF. Confirma o estado e reativa.";
      default:break;
    }
  }
  if(!heat.confirmed(8,false,millis())&&!heat.armed)
    return "A aguardar feedback OFF da bomba em "+groupText(hc.feedback[8])+" (comando "+groupText(hc.command[8])+").";
  if(!heat.armed)for(unsigned i=0;i<8;++i)if(heat.active(i)&&!heat.confirmed(i,false,millis()))
    return "A aguardar feedback OFF do circuito "+String(i+1)+" em "+groupText(hc.feedback[i])+".";
  if(!heat.armed)return "Pronto. Carrega em Ativar controlo e escolhe as divisoes.";
  return "Controlo ativo.";
}
void heatingTelegram(const knxip::Telegram &t) {
 if(!hc.enabled)return;
 uint32_t now=millis();bool b;
 if(t.operation!=knxip::Operation::Read){
  for(unsigned i=0;i<9;++i)if(heat.active(i)&&t.group==hc.feedback[i]&&t.asBool(b))heat.report(i,b,now);
  for(unsigned i=0;i<roomCount;++i){
   if(t.group==hc.enable[i]&&t.asBool(b)){zoneEnabled[i]=b;zoneMode[i]=0;roomStatusDirty[i]=true;heatCacheDirty=heatCacheUrgent=true;}
   if(!t.compact&&t.size==2){
    uint16_t raw=(uint16_t(t.data[0])<<8)|t.data[1];
    int mantissa=raw&2047;if(raw&32768)mantissa-=2048;
    float v=0.01f*mantissa*(1u<<((raw>>11)&15));
    if(raw==0x7fff){if(t.group==hc.temperature[i])hz[i].valid=false;if(t.group==hc.setpoint[i]){setpointReceived[i]=true;hz[i].setpointValid=false;if(heatCache[i].hasSetpoint){heatCache[i].hasSetpoint=false;heatCacheDirty=heatCacheUrgent=true;}}continue;}
    if(t.group==hc.temperature[i]){hz[i].temperature=v;hz[i].receivedAt=now;hz[i].valid=v>=-40&&v<=100;
      if(hz[i].valid){auto &c=heatCache[i];if(!c.hasTemperature||c.temperature!=v){heatCacheUrgent|=!c.hasTemperature;heatCacheDirty=true;c.temperature=v;c.hasTemperature=true;}}
    }
    if(t.group==hc.setpoint[i]){setpointReceived[i]=true;hz[i].setpoint=v;hz[i].setpointValid=v>=5&&v<=35;
      auto &c=heatCache[i];if(c.hasSetpoint!=hz[i].setpointValid||(hz[i].setpointValid&&c.setpoint!=v)){c.hasSetpoint=hz[i].setpointValid;c.setpoint=v;heatCacheDirty=heatCacheUrgent=true;}
    }
   }
  }
 }else for(unsigned i=0;i<roomCount;++i){if(t.group==hc.status[i])roomReadStatus[i]=true;if(t.group==hc.enable[i])roomReadCommand[i]=true;}
}
void heatingLoop(uint32_t now) {
 saveHeatingValues();
 if(!hc.enabled)return;
 bool link=tunnel.state()==knxip::State::Connected;
 for(unsigned i=0;i<roomCount;++i){
  hz[i].source=heating::TemperatureSource::Knx;hz[i].hysteresis=2*hc.margin[i];hz[i].maxAgeMs=hc.maxAgeMs;
  heating::defaultSetpoint(hz[i],setpointReceived[i],now,bootAt);
  effective[i]=heating::selectTemperature(hz[i],hc.bmeFallback[i],readings[0].value,
    config.bmeEnabled&&fresh(0,now),readings[0].at,now,bootAt);
  if(heating::manualExpired(zoneMode[i],now,zoneManualAt[i])){zoneMode[i]=1;zoneEnabled[i]=false;roomStatusDirty[i]=true;heatCacheDirty=heatCacheUrgent=true;}
  zoneDemand[i]=heating::roomDemand(heat.armed,zoneMode[i],zoneEnabled[i],hc.external[i],effective[i],now,zoneDemand[i]);
 }
 for(unsigned i=0;i<8;++i)heat.demand[i]=heatingLayout.zone[i]>=0&&zoneDemand[heatingLayout.zone[i]];
 heat.openMs=hc.openMs;heat.tick(now,link);
 if(!heat.armed)for(unsigned i=0;i<roomCount;++i)zoneDemand[i]=false;
 static bool hadFault=false;
 if(heat.fault&&!hadFault)Serial.println("Aquecimento: "+heatingBlockReason());
 hadFault=heat.fault;
 if(!link)return;
 for(unsigned i=0;i<roomCount;++i){
   uint8_t value=zoneEnabled[i];
   if(roomReadCommand[i]&&hc.enable[i]&&tunnel.respond(hc.enable[i],&value,1,true)==knxip::Result::Queued)roomReadCommand[i]=false;
   if(roomReadStatus[i]&&hc.status[i]&&tunnel.respond(hc.status[i],&value,1,true)==knxip::Result::Queued)roomReadStatus[i]=false;
   if(roomStatusDirty[i]&&hc.status[i]&&tunnel.writeBool(hc.status[i],value)==knxip::Result::Queued)roomStatusDirty[i]=false;
 }
 // Limit heating traffic and enqueue pump before valve commands.
 if(heating::age(now,heatSend)>=200){
  int next=heat.nextCommand(now);
  if(next>=0){unsigned i=unsigned(next);auto &a=heat.output[i];
   if(hc.command[i]&&tunnel.writeBool(hc.command[i],a.command,heatToken+1)==knxip::Result::Queued){heatPending[i]=++heatToken;a.sent=true;a.acknowledged=false;a.changed=now;heatSend=now;}
  }
 }
 if(heating::age(now,heatPoll)>=500){
  // A command may need one read of its own feedback. Never scan periodically.
  for(unsigned k=0;k<9;++k){
   unsigned i=(k+8)%9;
   if(!heat.active(i)){heatFeedbackRead[i]=false;continue;}
   if(!heatFeedbackRead[i])continue;
   if(heat.confirmed(i,heat.output[i].command,now)){heatFeedbackRead[i]=false;continue;}
   if(tunnel.read(hc.feedback[i])==knxip::Result::Queued){heatFeedbackRead[i]=false;heatPoll=now;}
   return;
  }
  if(!heatReadSweep.remaining)return;
  unsigned i=heatReadSweep.index;
  uint16_t g=i<9?hc.feedback[i]:i<9+roomCount?hc.temperature[i-9]:i<9+2*roomCount?hc.setpoint[i-9-roomCount]:hc.enable[i-9-2*roomCount];
  if(i<9&&!heat.active(i))g=0;
  if(!g||tunnel.read(g)==knxip::Result::Queued){heatReadSweep.complete();heatPoll=now;}
 }

}
void heatingJson(JsonDocument &d) {
  d["outputsEnabled"] = hc.enabled;
  d["canArm"]=hc.enabled&&heat.canArm(millis());d["faultOutput"]=heat.faultOutput;
  d["armed"]=heat.armed;d["fault"]=heat.fault;d["controlMessage"]=heatingBlockReason();d["valuesSaved"]=heatCacheSaveOk&&!heatCacheDirty;
  JsonObject control=d.createNestedObject("control");
  control["enabled"]=hc.enabled;control["openMs"]=hc.openMs;control["maxAgeMs"]=hc.maxAgeMs;
  JsonArray aa=control.createNestedArray("actuators");
  for(unsigned i=0;i<9;++i){auto a=aa.createNestedObject();a["command"]=hc.command[i]?groupText(hc.command[i]):"";a["feedback"]=hc.feedback[i]?groupText(hc.feedback[i]):"";}
  JsonArray rr=control.createNestedArray("rooms");
  for(unsigned i=0;i<roomCount;++i){auto r=rr.createNestedObject();r["enable"]=hc.enable[i]?groupText(hc.enable[i]):"";r["status"]=hc.status[i]?groupText(hc.status[i]):"";r["temperature"]=hc.temperature[i]?groupText(hc.temperature[i]):"";r["setpoint"]=hc.setpoint[i]?groupText(hc.setpoint[i]):"";r["external"]=hc.external[i];r["bmeFallback"]=hc.bmeFallback[i];r["margin"]=hc.margin[i];r["source"]=effective[i].valid?(effective[i].source==heating::TemperatureSource::Bme280?"BME local":"KNX"):"recurso / aguarda leitura";r["valid"]=effective[i].fresh(millis());r["value"]=effective[i].valid?effective[i].temperature:20;r["temperatureKnown"]=true;r["temperatureFresh"]=effective[i].valid;r["defaultTemperature"]=!effective[i].valid;r["setpointKnown"]=hz[i].setpointValid;r["target"]=hz[i].setpoint;r["mode"]=zoneMode[i];r["enabled"]=zoneEnabled[i];r["demand"]=zoneDemand[i];}
  auto state=[](unsigned i)->const char*{const auto&a=heat.output[i];if(!hc.enabled)return "desativado";if(!heat.active(i))return "nao utilizado";if(a.sent&&!heat.confirmed(i,a.command,millis()))return heating::age(millis(),a.changed)>=heat.timeoutMs?"sem confirmação":"pendente";if(!a.known)return "desconhecido";if(a.reported!=a.command)return "pendente";return a.reported?"ligado":"desligado";};
  d["pumpState"] = state(8);
  JsonArray zones = d.createNestedArray("zones");
  for (unsigned i=0;i<roomCount;++i) zones.add(heatingZones[i]);
  JsonArray circuits = d.createNestedArray("circuits");
  for (unsigned i = 0; i < 8; ++i) {
    JsonObject item = circuits.createNestedObject();
    item["position"] = int(heatingLayout.position[i]);
    item["zone"] = int(heatingLayout.zone[i]);
    item["state"] = state(i);
  }
}
bool parseHeating(JsonDocument &d, heating::RuntimeLayout &layout, String names[MaxRooms]) {
  if (!d["zones"].is<JsonArray>() || d["zones"].size() > MaxRooms ||
      !d["circuits"].is<JsonArray>() || (d["circuits"].size() != 8 && d["circuits"].size()!=7))
    return false;
  for (unsigned i = 0; i < d["zones"].size(); ++i) {
    if (!d["zones"][i].is<const char *>()) return false;
    names[i] = d["zones"][i].as<String>();
    names[i].trim();
    if (names[i].length() < 1 || names[i].length() > 32) return false;
  }
  for (unsigned i = 0; i < d["circuits"].size(); ++i) {
    JsonObject item = d["circuits"][i];
    if (!item["position"].is<int>() || !item["zone"].is<int>()) return false;
    int position = item["position"], zone = item["zone"];
    if (position < -1 || position >= 8 || zone < -1 || zone >= int(d["zones"].size()))
      return false;
    layout.position[i] = position;
    layout.zone[i] = zone;
  }
  if(d["circuits"].size()==7)layout.position[7]=-1;
  return layout.valid(d["zones"].size());
}
void loadHeating() {
  Preferences p;
  if (!p.begin("heating", true)) return;
  String serialized = p.getString("layout", "");
  p.end();
  if (serialized.isEmpty()) return;
  DynamicJsonDocument d(16384);
  if (deserializeJson(d, serialized)) return;
  heating::RuntimeLayout next;
  String names[MaxRooms];
  HeatConfig control;
  if (!parseHeating(d, next, names)||!parseHeatConfig(d,control)) return;
  hc=control;roomCount=d["zones"].size();
  heatingLayout = next;
  heat.configure(heatingLayout);
  for (unsigned i = 0; i < d["zones"].size(); ++i) heatingZones[i] = names[i];
}
void setupWeb() {
  const char *headers[] = {"X-CSRF-Token"};
  web.collectHeaders(headers, 1);
  web.on("/", HTTP_GET, [] {
    if (!auth()) return;
    web.sendHeader("Cache-Control", "no-store");
    web.sendHeader("X-Frame-Options", "DENY");
    web.send_P(200, "text/html; charset=utf-8", WEB_UI);
  });
  web.on("/api/heating/layout", HTTP_GET, [] {
    if (!auth()) return;
    DynamicJsonDocument d(16384);
    heatingJson(d);
    sendJson(d);
  });
  web.on("/api/heating/layout", HTTP_POST, [] {
    if (!mutation()) return;
    if (updating || restarting) { error(409, "Dispositivo ocupado"); return; }
    String body = web.arg("plain");
    if (body.length() > 16384) { error(413, "Mapa demasiado grande"); return; }
    DynamicJsonDocument d(16384);
    if (deserializeJson(d, body)) { error(400, "JSON invalido"); return; }
    heating::RuntimeLayout next;
    String names[MaxRooms];
    HeatConfig control;
    if (!heatingSafe()) { error(409,"Para o aquecimento e aguarda feedback da bomba antes de configurar"); return; }
    if (!parseHeating(d, next, names)||!parseHeatConfig(d,control)) {
      error(400, "Divisao, circuito ou posicao repetida/invalida"); return;
    }
    Preferences p;
    if (!p.begin("heating", false)) { error(500, "NVS indisponivel"); return; }
    bool saved = p.putString("layout", body) == body.length();
    p.end();
    if (!saved) { error(500, "Nao foi possivel guardar o mapa"); return; }
    saveHeatingValues(true);
    hc=control;roomCount=d["zones"].size();heat=heating::Runtime{};heatReadSweep.start(roomCount);
    for(auto &pending:heatFeedbackRead)pending=false;
    for(unsigned i=0;i<MaxRooms;++i){hz[i]={};zoneEnabled[i]=zoneDemand[i]=false;zoneMode[i]=0;setpointReceived[i]=false;effective[i]={};roomStatusDirty[i]=true;roomReadCommand[i]=roomReadStatus[i]=false;}
    heatingLayout = next;
    heat.configure(heatingLayout);
    for (unsigned i = 0; i < roomCount; ++i) heatingZones[i] = names[i];
    loadHeatingValues();
    DynamicJsonDocument response(16384);
    heatingJson(response);
    sendJson(response);
  });
  web.on("/api/heating/control",HTTP_POST,[]{
    if(!mutation())return;
    if(!hc.enabled||updating||restarting){error(409,"Aquecimento indisponivel");return;}
    StaticJsonDocument<256>d;
    if(web.arg("plain").length()>256||deserializeJson(d,web.arg("plain"))){error(400,"Pedido invalido");return;}
    String action=d["action"]|"";
    if(action=="refresh"){heatReadSweep.start(roomCount);heatPoll=millis()-500;}
    else if(action=="stop")heat.stop(millis());
    else if(action=="arm"){if(!heat.arm(millis())){error(409,heatingBlockReason());return;}for(unsigned i=0;i<roomCount;++i){roomStatusDirty[i]=true;}}
    else if(action=="room"&&heat.armed&&d["index"].is<unsigned>()&&d["mode"].is<int>()){
      unsigned i=d["index"];int mode=d["mode"];if(i>=roomCount||mode<0||mode>2){error(400,"Divisao/modo invalido");return;}zoneMode[i]=mode;zoneManualAt[i]=millis();zoneEnabled[i]=mode!=1;roomStatusDirty[i]=true;heatCacheDirty=heatCacheUrgent=true;
    }else{error(409,"Ativa primeiro o controlo");return;}
    web.send(200,"application/json","{\"ok\":true}");
  });
  web.on("/api/config", HTTP_GET, [] {
    if (!auth()) return;
    DynamicJsonDocument d(12288), c(10240);
    configJson(config, c, false);
    d["config"].set(c.as<JsonObject>());
    d["csrf"] = csrf;
    sendJson(d);
  });
  web.on("/api/config", HTTP_POST, [] {
    if (!mutation())
      return;
    if (updating || restarting) {
      error(409, "Dispositivo ocupado");
      return;
    }
    String body = web.arg("plain");
    if (body.length() > 10000) {
      error(413, "Configuracao demasiado grande");
      return;
    }
    DynamicJsonDocument d(12288);
    if (deserializeJson(d, body)) {
      error(400, "JSON invalido");
      return;
    }
    Config next;
    String why;
    if (!parseConfig(d, config, next, why)) {
      error(400, why);
      return;
    }
    if (!heatingSafe()) { error(409,"Para o aquecimento antes de reiniciar"); return; }
    if(hc.enabled){
      uint16_t groups[82];unsigned n=0;
      for(unsigned i=0;i<9;++i){groups[n++]=hc.command[i];groups[n++]=hc.feedback[i];}
      for(unsigned i=0;i<roomCount;++i){groups[n++]=hc.enable[i];groups[n++]=hc.status[i];groups[n++]=hc.temperature[i];groups[n++]=hc.setpoint[i];}
      for(unsigned i=0;i<n;++i){uint16_t g;
        for(const auto&r:next.relay)if(r.enabled&&((sensor::parseGroup(r.commandGroup.c_str(),g)&&g==groups[i])||(sensor::parseGroup(r.feedbackGroup.c_str(),g)&&g==groups[i]))){error(400,"Grupo reservado pelo aquecimento");return;}
        for(const auto&r:next.channel)if(r.enabled&&sensor::parseGroup(r.group.c_str(),g)&&g==groups[i]){error(400,"Grupo reservado pelo aquecimento");return;}
      }
    }
    if (!saveConfig(next)) {
      error(500, "Nao foi possivel guardar");
      return;
    }
    web.send(200, "application/json", "{\"saved\":true,\"reboot\":true}");
    scheduleRestart();
  });
  web.on("/api/relay", HTTP_POST, [] {
    if (!mutation()) return;
    if (updating || restarting) { error(409, "Dispositivo ocupado"); return; }
    if (web.arg("plain").length() > 128) { error(413, "Pedido demasiado grande"); return; }
    StaticJsonDocument<256> d;
    if (deserializeJson(d, web.arg("plain")) || !d["index"].is<int>() || !d["on"].is<bool>()) {
      error(400, "Indica index e on"); return;
    }
    int i = d["index"];
    if (i < 0 || i >= 4 || !config.relay[i].enabled) { error(400, "Rele indisponivel"); return; }
    setRelay(unsigned(i), d["on"].as<bool>());
    web.send(200, "application/json", "{\"applied\":true}");
  });
  web.on("/api/status", HTTP_GET, [] {
    if (!auth()) return;
    uint32_t now = millis();
    DynamicJsonDocument d(10240);
    d["name"] = config.name;
    d["firmware"] = "0.2.7-dashboard-order";
    d["ip"] =
        WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
    d["rssi"] = WiFi.RSSI();
    d["uptimeMs"] = now;
    d["freeHeap"] = ESP.getFreeHeap();
    d["knx"] = stateName();
    d["individual"] = individualText(tunnel.individualAddress());
    d["bme"] = bmeOk;
    d["oled"] = oledOk;
    d["i2cMain"] = bmeI2cScan;
    d["i2cOled"] = oledI2cScan;
    d["lastEvent"] = lastEvent;
    d["sent"] = tunnel.stats().sentAttempts;
    d["acked"] = tunnel.stats().acknowledged;
    d["failed"] = tunnel.stats().failed;
    JsonArray channels = d.createNestedArray("channels");
    for (unsigned i = 0; i < 5; ++i) {
      JsonObject o = channels.createNestedObject();
      o["name"] = config.channel[i].name;
      o["valid"] = fresh(i, now);
      if (fresh(i, now))
        o["value"] = readings[i].value;
      else
        o["value"] = nullptr;
      o["unit"] = i == 1 ? "%" : i == 2 ? "Pa" : "°C";
    }
    JsonArray outputs = d.createNestedArray("relays");
    for (unsigned i = 0; i < 4; ++i) {
      JsonObject o = outputs.createNestedObject();
      o["name"] = config.relay[i].name; o["enabled"] = config.relay[i].enabled;
      o["on"] = relays[i].on; o["gpio"] = config.relay[i].gpio;
      o["feedbackPending"] = relays[i].dirty || relays[i].pending || relays[i].readRequested;
    }
    JsonArray roms = d.createNestedArray("roms");
    for (unsigned i = 0; i < romCount; ++i)
      roms.add(detectedRom[i]);
    JsonArray t = d.createNestedArray("telegrams");
    for (unsigned i = 0; i < logCount; ++i)
      t.add(logs[(logNext + 12 - logCount + i) % 12]);
    sendJson(d);
  });
  web.on(
      "/api/update", HTTP_POST,
      [] {
        if (!mutation())
          return;
        if (!uploadAllowed) {
          error(409, "Atualizacao nao iniciada");
          return;
        }
        uploadAllowed = false;
        updating = false;
        if (!updateOk) {
          error(400, "Firmware invalido ou escrita falhou");
          return;
        }
        web.send(200, "application/json", "{\"updated\":true}");
        scheduleRestart();
      },
      [] {
        HTTPUpload &u = web.upload();
        if (u.status == UPLOAD_FILE_START) {
          uploadAllowed = false;
          updateOk = false;
          if (!authenticated() || web.header("X-CSRF-Token") != csrf || restarting || updating || !heatingSafe())
            return;
          saveHeatingValues(true);
          uploadAllowed = true;
          updating = true;
          lastUploadAt = millis();
          tunnel.stop(millis());
          updateOk = Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH);
        } else if (u.status == UPLOAD_FILE_WRITE) {
          lastUploadAt = millis();
          pollTunnel(lastUploadAt);
          if (uploadAllowed && updateOk && Update.write(u.buf, u.currentSize) != u.currentSize)
            updateOk = false;
        } else if (u.status == UPLOAD_FILE_END) {
          if (uploadAllowed) {
            if (updateOk)
              updateOk = Update.end(true);
            else
              Update.abort();
          }
        } else if (u.status == UPLOAD_FILE_ABORTED) {
          if (uploadAllowed) {
            Update.abort();
            updating = false;
            uploadAllowed = false;
            updateOk = false;
          }
        }
      });
  web.onNotFound([] {
    if (auth())
      error(404, "Nao encontrado");
  });
  web.begin();
}
void serialCommands() {
  while (Serial.available()) {
    char c = char(Serial.read());
    if (c == '\r')
      continue;
    if (c != '\n') {
      if (serialLine.length() < 100 && !serialOverflow)
        serialLine += c;
      else
        serialOverflow = true;
      continue;
    }
    String line = serialLine;
    serialLine = "";
    if (serialOverflow) {
      serialOverflow = false;
      Serial.println("Comando demasiado longo");
      continue;
    }
    line.trim();
    if (line == "HELP") {
      Serial.println("STATUS | SETUP | READ a/b/c | WRITE a/b/c 0|1 | FLOAT a/b/c valor | REBOOT");
    } else if (line == "STATUS") {
      Serial.println("IP " + WiFi.localIP().toString() + " KNX " + stateName());
    } else if (line == "SETUP") {
      openAp();
      Serial.println(String("Password AP: ") + (apPassword()[0] ? apPassword() : "(rede aberta)"));
      Serial.println("Web: sem login durante os testes");
    } else if (line == "REBOOT") {
      scheduleRestart();
    } else {
      char cmd[8] = {}, group[16] = {}, extra = 0;
      float value = 0;
      int count = sscanf(line.c_str(), "%7s %15s %f %c", cmd, group, &value, &extra);
      uint16_t g;
      knxip::Result result = knxip::Result::InvalidArgument;
      if (!updating && !restarting && sensor::parseGroup(group, g)) {
        if (count == 2 && !strcmp(cmd, "READ"))
          result = tunnel.read(g);
        else if (count == 3 && !strcmp(cmd, "WRITE") && (value == 0 || value == 1))
          result = tunnel.writeBool(g, value != 0);
        else if (count == 3 && !strcmp(cmd, "FLOAT")) {
          uint8_t b[2];
          if (sensor::encodeDpt9(value, b))
            result = tunnel.write(g, b, 2, false);
        }
      }
      Serial.println("Resultado: " + String(int(result)) +
                     " (0=fila, 1=desligado, 2=fila cheia, 3=invalido)");
    }
  }
}
} // namespace
void setup() {
  Serial.begin(115200);
  bootAt = millis();
  bool configured = loadConfig(config);
  Serial.println(configured ? "NVS: configuracao carregada" :
                              "NVS: configuracao ausente ou invalida; usar valores iniciais");
  if (!configured) {
    config.adminPassword = randomSecret();
    if (!saveConfig(config)) {
      Serial.println("ERRO NVS: configuracao nao persistida");
    }
    Serial.println("Primeiro acesso: admin / " + config.adminPassword);
  }
  // Private test defaults are only used until credentials have been saved in NVS.
  // Never overwrite a user's saved Web settings on every boot.
#ifdef LOCAL_WIFI_SSID
  if (config.ssid.isEmpty()) config.ssid = LOCAL_WIFI_SSID;
#endif
#ifdef LOCAL_WIFI_PASSWORD
  if (config.wifiPassword.isEmpty()) config.wifiPassword = LOCAL_WIFI_PASSWORD;
#endif
  loadHeating();
  loadHeatingValues();
  initRelays();
  csrf = randomSecret();
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(config.name.c_str());
  WiFi.setAutoReconnect(true);
  if (config.ssid.isEmpty())
    openAp();
  else {
    WiFi.begin(config.ssid.c_str(), config.wifiPassword.c_str());
    lastWifiAttempt = millis();
  }
  initSensors();
  setupWeb();
  Serial.println("Sensor KNX 0.2.7-dashboard-order. HELP para comandos; SETUP para acesso local.");
}
void loop() {
  uint32_t now = millis();
  if (updating && sensor::elapsed(now, lastUploadAt, 60000)) {
    Update.abort();
    updating = false;
    uploadAllowed = false;
    updateOk = false;
  }
  network(now);
  pollTunnel(now);
  web.handleClient();
  serialCommands();
  if (!updating && !restarting) {
    sampleSensors(millis());
    heatingLoop(millis());
    publishRelays(millis());
    publish(millis());
    display(millis());
  }
  pollTunnel(millis());
  if (restarting &&
      (tunnel.state() == knxip::State::Stopped || sensor::elapsed(millis(), restartAt, 11000)) &&
      sensor::elapsed(millis(), restartAt, 1500))
    ESP.restart();
  delay(1);
}
