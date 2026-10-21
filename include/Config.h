#pragma once
#include "SensorCore.h"
#include "RelayCore.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
struct ChannelConfig {
  String name, group;
  bool enabled = false;
  float offset = 0;
  sensor::Policy policy;
};
struct RelayConfig {
  String name, commandGroup, feedbackGroup;
  bool enabled = false, activeLow = true;
  int gpio = -1;
};
struct Config {
  String name = "iot-sensor", ssid, wifiPassword, adminPassword;
  bool knxEnabled = false, bmeEnabled = false, oledEnabled = false;
  String gateway;
  uint16_t port = 3671, rate = 5;
  int sda = -1, scl = -1, oneWire = -1, oledReset = -1, oledHeight = 64;
  uint8_t bmeAddress = 0x76, oledAddress = 0x3c;
  uint32_t sampleMs = 5000;
  String rom[2];
  ChannelConfig channel[5];
  RelayConfig relay[4];
  Config() {
    for (int i = 0; i < 4; ++i) relay[i].name = "Rele " + String(i + 1);
    const char *names[] = {"Ambiente", "Humidade", "Pressao", "Sonda 1", "Sonda 2"};
    for (int i = 0; i < 5; ++i) {
      channel[i].name = names[i];
      channel[i].policy.delta = i == 1 ? 1.0f : i == 2 ? 100.0f : 0.2f;
    }
  }
};
extern Config config;
void configJson(const Config &, JsonDocument &, bool secrets);
bool parseConfig(JsonDocument &, const Config &, Config &, String &);
bool saveConfig(const Config &);
bool loadConfig(Config &);
String randomSecret();
