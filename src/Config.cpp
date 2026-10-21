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
static bool pin(int p) {
  return p == 4 || p == 13 || p == 14 || p == 15 || p == 16 || p == 17 || p == 18 || p == 19 ||
         p == 21 || p == 22 || p == 23 || p == 25 || p == 26 || p == 27 || p == 32 || p == 33;
}
void configJson(const Config &c, JsonDocument &d, bool secrets) {
  d["schema"] = 1;
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
  d["sda"] = c.sda;
  d["scl"] = c.scl;
  d["oneWire"] = c.oneWire;
  d["oledReset"] = c.oledReset;
  d["oledHeight"] = c.oledHeight;
  d["bmeAddress"] = c.bmeAddress;
  d["oledAddress"] = c.oledAddress;
  d["sampleMs"] = c.sampleMs;
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
  if (d["schema"] != 1 || !d["channels"].is<JsonArray>() || d["channels"].size() != 5 ||
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
  c.oneWire = d["oneWire"];
  c.oledReset = d["oledReset"];
  c.oledHeight = d["oledHeight"];
  if ((c.sda != -1 && !pin(c.sda)) || (c.scl != -1 && !pin(c.scl)) ||
      (c.oneWire != -1 && !pin(c.oneWire)) || (c.oledReset != -1 && !pin(c.oledReset)))
    return bad("GPIO nao permitido neste perfil ESP32");
  if ((c.bmeEnabled || c.oledEnabled) && (!pin(c.sda) || !pin(c.scl) || c.sda == c.scl))
    return bad("Configura os pinos I2C");
  if (c.oneWire >= 0 && (c.oneWire == c.sda || c.oneWire == c.scl || c.oneWire == c.oledReset))
    return bad("Conflito de GPIO OneWire");
  if (c.oledReset >= 0 && (c.oledReset == c.sda || c.oledReset == c.scl))
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
  return true;
}
bool saveConfig(const Config &c) {
  DynamicJsonDocument d(6144);
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
  DynamicJsonDocument d(6144);
  if (deserializeJson(d, s))
    return false;
  String err;
  Config parsed;
  if (!parseConfig(d, c, parsed, err))
    return false;
  c = parsed;
  return true;
}
