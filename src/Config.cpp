#include "Config.h"
#include <WiFi.h>
#include <cctype>
#include <initializer_list>
Config config;
String randomSecret() {
  char b[25];
  snprintf(b, sizeof(b), "%08lx%08lx%08lx", (unsigned long)esp_random(),
           (unsigned long)esp_random(), (unsigned long)esp_random());
  return String(b);
}
static bool pin(int p) { return device::outputPin(p); }
void configJson(const Config &c, JsonDocument &d, bool secrets) {
  d["schema"] = 2;
  d["name"] = c.name;
  d["ssid"] = c.ssid;
  if (secrets) {
    d["wifiPassword"] = c.wifiPassword;
    d["adminPassword"] = c.adminPassword;
  }
  d["knxEnabled"] = c.knxEnabled;
  d["gateway"] = c.gateway;
  d["port"] = c.port;
  d["rate"] = c.rate;
  d["bmeEnabled"] = c.bmeEnabled;
  d["oledEnabled"] = c.oledEnabled;
  d["showDiagnostics"] = c.showDiagnostics;
  d["sda"] = c.sda;
  d["scl"] = c.scl;
  d["oledSda"] = c.oledSda;
  d["oledScl"] = c.oledScl;
  d["oneWire"] = c.oneWire;
  d["oledReset"] = c.oledReset;
  d["oledHeight"] = c.oledHeight;
  d["bmeAddress"] = c.bmeAddress;
  d["oledAddress"] = c.oledAddress;
  d["sampleMs"] = c.sampleMs;
  JsonArray relays = d.createNestedArray("relays");
  for (const auto &r : c.relay) {
    JsonObject o = relays.createNestedObject();
    o["name"] = r.name; o["enabled"] = r.enabled; o["gpio"] = r.gpio;
    o["activeLow"] = r.activeLow;
    o["commandGroup"] = r.commandGroup; o["feedbackGroup"] = r.feedbackGroup;
  }
  JsonArray rom = d.createNestedArray("rom");
  rom.add(c.rom[0]);
  rom.add(c.rom[1]);
  JsonArray a = d.createNestedArray("channels");
  for (auto &ch : c.channel) {
    JsonObject o = a.createNestedObject();
    o["name"] = ch.name;
    o["enabled"] = ch.enabled;
    o["group"] = ch.group;
    o["offset"] = ch.offset;
    o["periodic"] = ch.policy.periodic;
    o["onChange"] = ch.policy.onChange;
    o["periodMs"] = ch.policy.periodMs;
    o["minGapMs"] = ch.policy.minGapMs;
    o["delta"] = ch.policy.delta;
  }
}
bool parseConfig(JsonDocument &d, const Config &old, Config &c, String &err) {
  auto bad = [&](const char *s) {
    err = s;
    return false;
  };
  c = old;
  if ((d["schema"] != 1 && d["schema"] != 2) || !d["channels"].is<JsonArray>() || d["channels"].size() != 5 ||
      !d["rom"].is<JsonArray>() || d["rom"].size() != 2)
    return bad("Formato de configuracao invalido");
  const char *strings[] = {"name", "ssid", "gateway"};
  for (auto k : strings)
    if (!d[k].is<const char *>())
      return bad("Campo de texto em falta");
  const char *flags[] = {"knxEnabled", "bmeEnabled", "oledEnabled"};
  for (auto k : flags)
    if (!d[k].is<bool>())
      return bad("Opcao invalida");
  const char *ints[] = {"port",      "rate",       "sda",        "scl",         "oneWire",
                        "oledReset", "oledHeight", "bmeAddress", "oledAddress", "sampleMs"};
  for (auto k : ints)
    if (!d[k].is<int>())
      return bad("Numero inteiro invalido");
  c.name = d["name"].as<String>();
  c.ssid = d["ssid"].as<String>();
  c.gateway = d["gateway"].as<String>();
  if (c.name.length() < 1 || c.name.length() > 32)
    return bad("Nome: 1 a 32 caracteres");
  if (!isalnum((unsigned char)c.name[0]) || !isalnum((unsigned char)c.name[c.name.length() - 1]))
    return bad("Nome deve comecar e terminar com letra ou numero");
  for (char x : c.name)
    if (!isalnum((unsigned char)x) && x != '-')
      return bad("Nome: letras, numeros e hifen");
  if (c.ssid.length() > 32)
    return bad("SSID demasiado longo");
  for (auto k : {"wifiPassword", "adminPassword"})
    if (d.containsKey(k) && !d[k].is<const char *>())
      return bad("Password invalida");
  String wp = d["wifiPassword"] | "", ap = d["adminPassword"] | "";
  if (wp.length())
    c.wifiPassword = wp;
  if (ap.length())
    c.adminPassword = ap;
  if (c.wifiPassword.length() > 63 || (c.wifiPassword.length() > 0 && c.wifiPassword.length() < 8))
    return bad("Password Wi-Fi: 8 a 63 caracteres");
  if (c.adminPassword.length() < 12 || c.adminPassword.length() > 63)
    return bad("Password de administracao: 12 a 63 caracteres");
  c.knxEnabled = d["knxEnabled"];
  c.bmeEnabled = d["bmeEnabled"];
  c.oledEnabled = d["oledEnabled"];
  if (d.containsKey("showDiagnostics") && !d["showDiagnostics"].is<bool>())
    return bad("Opcao de diagnostico invalida");
  c.showDiagnostics = d["showDiagnostics"] | true;
  int port = d["port"], rate = d["rate"], sample = d["sampleMs"];
  if (port < 1 || port > 65535 || rate < 1 || rate > 20 || sample < 2000 || sample > 60000)
    return bad("Porta, limite KNX ou intervalo de leitura invalido");
  c.port = port;
  c.rate = rate;
  c.sampleMs = sample;
  IPAddress ip;
  if (c.knxEnabled && (!ip.fromString(c.gateway) || ip[0] == 0 || ip[0] >= 224 ||
                       ip == IPAddress(255, 255, 255, 255)))
    return bad("IPv4 do servidor KNX invalido");
  c.sda = d["sda"];
  c.scl = d["scl"];
  c.oledSda = d.containsKey("oledSda") ? d["oledSda"].as<int>() : -1;
  c.oledScl = d.containsKey("oledScl") ? d["oledScl"].as<int>() : -1;
  if ((d.containsKey("oledSda") && !d["oledSda"].is<int>()) ||
      (d.containsKey("oledScl") && !d["oledScl"].is<int>()))
    return bad("Pinos OLED invalidos");
  c.oneWire = d["oneWire"];
  c.oledReset = d["oledReset"];
  c.oledHeight = d["oledHeight"];
  if ((c.sda != -1 && !pin(c.sda)) || (c.scl != -1 && !pin(c.scl)) ||
      (c.oledSda != -1 && !pin(c.oledSda)) || (c.oledScl != -1 && !pin(c.oledScl)) ||
      (c.oneWire != -1 && !pin(c.oneWire)) || (c.oledReset != -1 && !pin(c.oledReset)))
    return bad("GPIO nao permitido neste perfil ESP32");
  if (c.bmeEnabled && (!pin(c.sda) || !pin(c.scl) || c.sda == c.scl))
    return bad("Configura os pinos I2C");
  if ((c.oledSda == -1) != (c.oledScl == -1))
    return bad("Configura os dois pinos OLED ou deixa ambos em -1");
  int osda = c.oledSda == -1 ? c.sda : c.oledSda;
  int oscl = c.oledScl == -1 ? c.scl : c.oledScl;
  if (c.oledEnabled && (!pin(osda) || !pin(oscl) || osda == oscl))
    return bad("Configura os pinos I2C do OLED");
  if (c.bmeEnabled && c.oledEnabled && c.oledSda != -1 &&
      (osda == c.sda || osda == c.scl || oscl == c.sda || oscl == c.scl))
    return bad("Barramentos I2C separados nao podem partilhar GPIO");
  if (c.oneWire >= 0 && ((c.bmeEnabled && (c.oneWire == c.sda || c.oneWire == c.scl)) ||
      (c.oledEnabled && (c.oneWire == osda || c.oneWire == oscl)) || c.oneWire == c.oledReset))
    return bad("Conflito de GPIO OneWire");
  if (c.oledEnabled && c.oledReset >= 0 && (c.oledReset == osda || c.oledReset == oscl ||
      (c.bmeEnabled && (c.oledReset == c.sda || c.oledReset == c.scl))))
    return bad("Conflito de GPIO OLED");
  if (c.oledHeight != 32 && c.oledHeight != 64)
    return bad("Altura OLED: 32 ou 64");
  int ba = d["bmeAddress"], oa = d["oledAddress"];
  if ((ba != 0x76 && ba != 0x77) || (oa != 0x3c && oa != 0x3d))
    return bad("Endereco I2C invalido");
  c.bmeAddress = ba;
  c.oledAddress = oa;
  for (int i = 0; i < 2; ++i) {
    if (!d["rom"][i].is<const char *>())
      return bad("ROM invalido");
    c.rom[i] = d["rom"][i].as<String>();
    c.rom[i].toUpperCase();
    uint8_t r[8];
    if (c.rom[i].length() && !sensor::parseRom(c.rom[i].c_str(), r))
      return bad("ROM DS18B20 ou CRC invalido");
  }
  if (c.rom[0].length() && c.rom[0] == c.rom[1])
    return bad("As sondas devem ter ROM diferentes");
  uint16_t groups[5] = {};
  for (int i = 0; i < 5; ++i) {
    JsonObject o = d["channels"][i];
    auto &ch = c.channel[i];
    if (!o["name"].is<const char *>() || !o["group"].is<const char *>() ||
        !o["enabled"].is<bool>() || !o["periodic"].is<bool>() || !o["onChange"].is<bool>() ||
        !o["offset"].is<float>() || !o["delta"].is<float>() || !o["periodMs"].is<int>() ||
        !o["minGapMs"].is<int>())
      return bad("Campos do canal invalidos");
    ch.name = o["name"].as<String>();
    ch.group = o["group"].as<String>();
    ch.enabled = o["enabled"];
    ch.offset = o["offset"];
    ch.policy.periodic = o["periodic"];
    ch.policy.onChange = o["onChange"];
    int period = o["periodMs"], gap = o["minGapMs"];
    ch.policy.delta = o["delta"];
    if (ch.name.length() < 1 || ch.name.length() > 32 || !std::isfinite(ch.offset) ||
        fabs(ch.offset) > (i == 2 ? 10000 : 20) || !std::isfinite(ch.policy.delta) ||
        ch.policy.delta <= 0 || period < 2000 || period > 86400000 || gap < 1000 || gap > 3600000 ||
        gap > period)
      return bad("Parametros do canal fora dos limites");
    ch.policy.periodMs = period;
    ch.policy.minGapMs = gap;
    if (ch.group.length() && !sensor::parseGroup(ch.group.c_str(), groups[i]))
      return bad("Grupo invalido: principal 0-15, intermedio 0-7, subgrupo 0-255");
    if (ch.enabled) {
      if (!groups[i])
        return bad("Canal ativo exige grupo KNX");
      if (i < 3 && !c.bmeEnabled)
        return bad("Ativar BME280 antes do canal");
      if (i >= 3 && (c.oneWire < 0 || c.rom[i - 3].isEmpty()))
        return bad("Canal de sonda exige GPIO e ROM");
      for (int j = 0; j < i; ++j)
        if (c.channel[j].enabled && groups[i] == groups[j])
          return bad("Grupos de medicao devem ser diferentes");
    }
  }
  // Migrate schema 1 with disabled outputs; schema 2 always includes all four relays.
  if (d["schema"] == 1) {
    for (unsigned i = 0; i < 4; ++i) {
      c.relay[i] = RelayConfig();
      c.relay[i].name = "Rele " + String(i + 1);
    }
  } else {
    if (!d["relays"].is<JsonArray>() || d["relays"].size() != 4)
      return bad("Configura exatamente quatro reles");
    uint16_t commands[4] = {}, feedbacks[4] = {};
    for (unsigned i = 0; i < 4; ++i) {
      JsonObject o = d["relays"][i];
      auto &r = c.relay[i];
      if (!o["name"].is<const char *>() || !o["enabled"].is<bool>() ||
          !o["gpio"].is<int>() || !o["activeLow"].is<bool>() ||
          !o["commandGroup"].is<const char *>() || !o["feedbackGroup"].is<const char *>())
        return bad("Campos do rele invalidos");
      r.name = o["name"].as<String>(); r.enabled = o["enabled"];
      r.gpio = o["gpio"]; r.activeLow = o["activeLow"];
      r.commandGroup = o["commandGroup"].as<String>();
      r.feedbackGroup = o["feedbackGroup"].as<String>();
      if (r.name.length() < 1 || r.name.length() > 32 ||
          (r.gpio != -1 && !pin(r.gpio))) return bad("Nome ou GPIO do rele invalido");
      if ((r.commandGroup.length() && !sensor::parseGroup(r.commandGroup.c_str(), commands[i])) ||
          (r.feedbackGroup.length() && !sensor::parseGroup(r.feedbackGroup.c_str(), feedbacks[i])))
        return bad("Grupo KNX do rele invalido");
      if (!r.enabled) continue;
      if (!pin(r.gpio) || !commands[i] || !feedbacks[i])
        return bad("Rele ativo exige GPIO e grupos de comando e feedback");
      if (commands[i] == feedbacks[i]) return bad("Comando e feedback devem ser diferentes");
      if ((c.bmeEnabled && (r.gpio == c.sda || r.gpio == c.scl)) ||
          (c.oledEnabled && (r.gpio == osda || r.gpio == oscl)) ||
          r.gpio == c.oneWire || (c.oledEnabled && r.gpio == c.oledReset))
        return bad("GPIO do rele em conflito com sensor ou OLED");
      for (unsigned j = 0; j < 5; ++j)
        if (c.channel[j].enabled && (groups[j] == commands[i] || groups[j] == feedbacks[i]))
          return bad("Grupos de rele em conflito com medicao");
      for (unsigned j = 0; j < i; ++j) {
        if (!c.relay[j].enabled) continue;
        if (r.gpio == c.relay[j].gpio) return bad("GPIO repetido entre reles");
        if (commands[i] == commands[j] || commands[i] == feedbacks[j] ||
            feedbacks[i] == commands[j] || feedbacks[i] == feedbacks[j])
          return bad("Cada rele deve ter grupos KNX exclusivos");
      }
    }
  }
  return true;
}
bool saveConfig(const Config &c) {
  DynamicJsonDocument d(10240);
  configJson(c, d, true);
  if (d.overflowed())
    return false;
  String s;
  serializeJson(d, s);
  Preferences p;
  if (!p.begin("sensor", false))
    return false;
  bool ok = p.putString("config", s) == s.length();
  p.end();
  return ok;
}
bool loadConfig(Config &c) {
  Preferences p;
  if (!p.begin("sensor", true))
    return false;
  String s = p.getString("config", "");
  p.end();
  if (s.isEmpty())
    return false;
  DynamicJsonDocument d(10240);
  if (deserializeJson(d, s))
    return false;
  String err;
  Config parsed;
  if (!parseConfig(d, c, parsed, err))
    return false;
  c = parsed;
  return true;
}
