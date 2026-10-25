#pragma once
#include <array>
#include <cstdint>

class TwoWire {
 public:
  std::array<uint8_t, 256> registers{};
  unsigned modeWrites = 0, burstReads = 0;
  bool rejectData = false;
  void beginTransmission(uint8_t address) { currentAddress = address; written = 0; }
  void write(uint8_t value) { if (written++ == 0) registerAddress = value; else writeValue = value; }
  uint8_t endTransmission(bool) {
    if (currentAddress != 0x77) return 2;
    if (written == 2) {
      if (registerAddress == 0xF4 && ++modeWrites > 1) return 2;
      registers[registerAddress] = writeValue;
    }
    return 0;
  }
  uint8_t requestFrom(uint8_t address, uint8_t count) {
    if (address != 0x77 || (count == 8 && rejectData)) return 0;
    if (count == 8) ++burstReads;
    remaining = count;
    return count;
  }
  int read() { --remaining; return registers[registerAddress++]; }
 private:
  uint8_t currentAddress = 0, registerAddress = 0, writeValue = 0, written = 0, remaining = 0;
};
