#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <cmath>

// Uses the already configured TwoWire instance. In particular, it never calls
// Wire.begin() again, which would interfere with a configurable SDA pin.
class Bme280Direct {
 public:
  bool begin(uint8_t address, TwoWire &bus) {
    bus_ = &bus;
    address_ = address;
    ready_ = false;
    error_ = "ID I2C";
    uint8_t id;
    if (!read(0xD0, &id, 1) || id != 0x60) return false;
    error_ = "calibracao I2C";
    uint8_t trim[26], humidity[7];
    if (!read(0x88, trim, sizeof(trim)) || !read(0xE1, humidity, sizeof(humidity)))
      return false;
    t1_ = u16(trim); t2_ = s16(trim + 2); t3_ = s16(trim + 4);
    p1_ = u16(trim + 6); p2_ = s16(trim + 8); p3_ = s16(trim + 10);
    p4_ = s16(trim + 12); p5_ = s16(trim + 14); p6_ = s16(trim + 16);
    p7_ = s16(trim + 18); p8_ = s16(trim + 20); p9_ = s16(trim + 22);
    h1_ = trim[25]; h2_ = s16(humidity); h3_ = humidity[2];
    h4_ = sign12((uint16_t(humidity[3]) << 4) | (humidity[4] & 0x0f));
    h5_ = sign12((uint16_t(humidity[5]) << 4) | (humidity[4] >> 4));
    h6_ = int8_t(humidity[6]);
    error_ = "calibracao invalida";
    if (!t1_ || !p1_) return false;
    // Humidity oversampling must precede ctrl_meas to take effect.
    error_ = "configuracao I2C";
    if (!write(0xF2, 0x01) || !write(0xF5, 0x00) || !write(0xF4, 0x24))
      return false; // sleep mode, x1 temperature and pressure
    ready_ = true;
    return true;
  }

  bool sample(float &temperature, float &humidity, float &pressure) {
    if (!ready_) return false;
    // Each forced conversion is a single coherent sample. At x1/x1/x1 the
    // Bosch worst-case conversion time is 9.3 ms; wait, then check measuring.
    error_ = "iniciar medicao";
    if (!write(0xF4, 0x25)) { ready_ = false; return false; }
    delay(10);
    error_ = "aguardar medicao";
    uint8_t status = 0;
    bool finished = false;
    for (unsigned attempt = 0; attempt < 20; ++attempt) {
      if (!read(0xF3, &status, 1)) { ready_ = false; return false; }
      if ((status & 0x08) == 0) { finished = true; break; }
      delay(1);
    }
    if (!finished) return false;
    uint8_t raw[8];
    error_ = "dados I2C";
    if (!read(0xF7, raw, sizeof(raw))) { ready_ = false; return false; }
    int32_t adcP = (int32_t(raw[0]) << 12) | (int32_t(raw[1]) << 4) | (raw[2] >> 4);
    int32_t adcT = (int32_t(raw[3]) << 12) | (int32_t(raw[4]) << 4) | (raw[5] >> 4);
    int32_t adcH = (int32_t(raw[6]) << 8) | raw[7];
    error_ = "ADC sem medicao";
    if (adcP == 0x80000 || adcT == 0x80000 || adcH == 0x8000) return false;
    // Bosch BME280 compensation in double precision; pressure is in Pa.
    double v1 = (adcT / 16384.0 - t1_ / 1024.0) * t2_;
    double v2 = (adcT / 131072.0 - t1_ / 8192.0);
    v2 = v2 * v2 * t3_;
    double fine = v1 + v2;
    temperature = float(fine / 5120.0);
    v1 = fine / 2.0 - 64000.0;
    v2 = v1 * v1 * p6_ / 32768.0;
    v2 += v1 * p5_ * 2.0;
    v2 = v2 / 4.0 + p4_ * 65536.0;
    v1 = (p3_ * v1 * v1 / 524288.0 + p2_ * v1) / 524288.0;
    v1 = (1.0 + v1 / 32768.0) * p1_;
    error_ = "pressao sem calibracao";
    if (v1 == 0.0) return false;
    double p = 1048576.0 - adcP;
    p = (p - v2 / 4096.0) * 6250.0 / v1;
    v1 = p9_ * p * p / 2147483648.0;
    v2 = p * p8_ / 32768.0;
    pressure = float(p + (v1 + v2 + p7_) / 16.0);
    double h = fine - 76800.0;
    h = (adcH - (h4_ * 64.0 + h5_ / 16384.0 * h)) *
        (h2_ / 65536.0 * (1.0 + h6_ / 67108864.0 * h *
        (1.0 + h3_ / 67108864.0 * h)));
    h *= 1.0 - h1_ * h / 524288.0;
    humidity = float(constrain(h, 0.0, 100.0));
    error_ = "valor nao finito";
    return std::isfinite(temperature) && std::isfinite(humidity) && std::isfinite(pressure);
  }

  const char *lastError() const { return error_; }
  uint8_t lastRegister() const { return lastRegister_; }

 private:
  static uint16_t u16(const uint8_t *p) { return uint16_t(p[0]) | (uint16_t(p[1]) << 8); }
  static int16_t s16(const uint8_t *p) { return int16_t(u16(p)); }
  static int16_t sign12(uint16_t v) { return (v & 0x800) ? int16_t(v | 0xf000) : int16_t(v); }
  bool write(uint8_t reg, uint8_t value) {
    lastRegister_ = reg;
    bus_->beginTransmission(address_);
    bus_->write(reg); bus_->write(value);
    return bus_->endTransmission() == 0;
  }
  bool read(uint8_t reg, uint8_t *bytes, size_t count) {
    // Small transfers avoid long I2C reads. End the register-select write
    // with STOP before reading; this board fails non-stop write/read requests.
    for (size_t offset = 0; offset < count; offset += 2) {
      uint8_t part = uint8_t(count - offset >= 2 ? 2 : 1);
      lastRegister_ = uint8_t(reg + offset);
      bus_->beginTransmission(address_);
      bus_->write(lastRegister_);
      if (bus_->endTransmission(true) != 0) return false;
      if (bus_->requestFrom(address_, part) != part) return false;
      for (uint8_t i = 0; i < part; ++i) bytes[offset + i] = bus_->read();
    }
    return true;
  }
  TwoWire *bus_ = nullptr;
  uint8_t address_ = 0;
  bool ready_ = false;
  const char *error_ = "por inicializar";
  uint8_t lastRegister_ = 0;
  uint16_t t1_, p1_;
  int16_t t2_, t3_, p2_, p3_, p4_, p5_, p6_, p7_, p8_, p9_;
  uint8_t h1_, h3_;
  int16_t h2_, h4_, h5_;
  int8_t h6_;
};
