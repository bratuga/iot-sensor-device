#include "Bme280Direct.h"
#include <cassert>
#include <cmath>
#include <iostream>

static void word(TwoWire &bus, uint8_t address, int16_t value) {
  bus.registers[address] = uint8_t(value);
  bus.registers[address + 1] = uint8_t(uint16_t(value) >> 8);
}
static void adc20(TwoWire &bus, uint8_t address, uint32_t value) {
  bus.registers[address] = uint8_t(value >> 12);
  bus.registers[address + 1] = uint8_t(value >> 4);
  bus.registers[address + 2] = uint8_t(value << 4);
}
int main() {
  TwoWire bus;
  bus.registers[0xD0] = 0x60;
  word(bus, 0x88, 27504); word(bus, 0x8A, 26435); word(bus, 0x8C, -1000);
  word(bus, 0x8E, 36477); word(bus, 0x90, -10685); word(bus, 0x92, 3024);
  word(bus, 0x94, 2855); word(bus, 0x96, 140); word(bus, 0x98, -7);
  word(bus, 0x9A, 15500); word(bus, 0x9C, -14600); word(bus, 0x9E, 6000);
  bus.registers[0xA1] = 75; word(bus, 0xE1, 362);
  bus.registers[0xE3] = 0; bus.registers[0xE4] = 0x14;
  bus.registers[0xE5] = 0x2E; bus.registers[0xE6] = 0x03;
  bus.registers[0xE7] = 30;
  adc20(bus, 0xF7, 415148); adc20(bus, 0xFA, 519888);
  bus.registers[0xFD] = 0x75; bus.registers[0xFE] = 0x30;

  Bme280Direct bme;
  assert(bme.begin(0x77, bus));
  assert(bus.registers[0xF2] == 1 && bus.registers[0xF5] == 0xA0);
  assert(bus.registers[0xF4] == 0x27 && bus.modeWrites == 1);
  float t = NAN, h = NAN, p = NAN;
  assert(bme.sample(t, h, p));
  assert(t > 20 && t < 30 && h >= 0 && h <= 100 && p > 90000 && p < 110000);
  adc20(bus, 0xFA, 525000);
  assert(bme.sample(t, h, p) && t > 20 && t < 30);
  assert(bus.modeWrites == 1 && bus.burstReads == 2);
  bus.rejectData = true;
  assert(!bme.sample(t, h, p) && bme.lastRegister() == 0xF7);
  std::cout << "BME280 normal mode: coherent burst, no repeated ctrl_meas writes\n";
}
