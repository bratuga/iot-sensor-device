#include "Config.h"
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
    Wire.setTimeOut(25);
    bmeI2cScan = scanI2c(Wire);
    if (config.bmeEnabled) {
      Serial.println("BME ID repeated-start: " + bmeChipId(Wire, true));
      Serial.println("BME ID com STOP: " + bmeChipId(Wire, false));
    }
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
bool authenticated() {
  return web.authenticate("admin", apPassword());
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
void setupWeb() {
  const char *headers[] = {"X-CSRF-Token"};
  web.collectHeaders(headers, 1);
  web.on("/", HTTP_GET, [] {
    if (!auth()) return;
    web.sendHeader("Cache-Control", "no-store");
    web.sendHeader("X-Frame-Options", "DENY");
    web.send_P(200, "text/html; charset=utf-8", WEB_UI);
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
    d["firmware"] = "0.2.0";
    d["name"] = config.name;
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
          if (!authenticated() || web.header("X-CSRF-Token") != csrf || restarting || updating)
            return;
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
      Serial.println(String("Password admin Web: ") + apPassword());
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
  Serial.println("Sensor KNX 0.2.0. HELP para comandos; SETUP para acesso local.");
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
