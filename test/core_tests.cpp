#include "SensorCore.h"
#include <cassert>
#include <iostream>
#include <limits>
static float decode(const uint8_t b[2]) {
  unsigned raw = (unsigned(b[0]) << 8) | b[1];
  int m = raw & 2047;
  if (raw & 0x8000)
    m -= 2048;
  return float(m) * float(1u << ((raw >> 11) & 15)) * 0.01f;
}
int main() {
  uint8_t b[2];
  assert(sensor::encodeDpt9(0, b) && b[0] == 0 && b[1] == 0);
  assert(sensor::encodeDpt9(20, b) && b[0] == 7 && b[1] == 0xd0);
  assert(sensor::encodeDpt9(-5, b) && b[0] == 0x86 && b[1] == 0x0c);
  assert(sensor::encodeDpt9(25, b) && b[0] == 0x0c && b[1] == 0xe2);
  assert(sensor::encodeDpt9(-20.48f, b) && b[0] == 0x80 && b[1] == 0);
  for (float f : {-55.0f, -0.01f, 0.01f, 20.47f, 20.48f, 50.f, 125.f, 101325.f}) {
    assert(sensor::encodeDpt9(f, b));
    float step = 0.01f * float(1u << ((b[0] >> 3) & 15));
    assert(std::fabs(decode(b) - f) <= step * 0.51f + 0.001f);
  }
  assert(!sensor::encodeDpt9(NAN, b));
  assert(!sensor::encodeDpt9(INFINITY, b));
  assert(!sensor::encodeDpt9(700000, b));
  uint16_t g;
  assert(sensor::parseGroup("15/7/255", g) && g == 32767);
  assert(sensor::parseGroup("1/2/3", g) && g == 2563);
  for (auto s : {"0/0/0", "16/0/0", "31/7/255", "32/0/0", "1/8/0", "1/1/256", "1/2/3x", "-1/2/3",
                 "1/2", "", "1/2/3/4"})
    assert(!sensor::parseGroup(s, g));
  uint8_t rom[8] = {0x28, 1, 2, 3, 4, 5, 6, 0};
  rom[7] = sensor::crc8(rom, 7);
  char r[17];
  for (unsigned i = 0; i < 8; ++i)
    snprintf(r + 2 * i, 3, "%02X", rom[i]);
  uint8_t parsed[8];
  assert(sensor::parseRom(r, parsed));
  r[15] = r[15] == '0' ? '1' : '0';
  assert(!sensor::parseRom(r, parsed));
  assert(!sensor::parseRom("28010203040506XX", parsed));
  sensor::Policy p;
  p.delta = 0.5f;
  p.periodMs = 60000;
  p.minGapMs = 5000;
  sensor::Publication pub;
  assert(pub.due(20, 0, p));
  pub.queued(20, 0);
  assert(!pub.due(21, 10000, p));
  pub.complete(true, 100);
  assert(!pub.due(20, 5000, p));
  assert(pub.due(20.5f, 5000, p));
  assert(pub.due(20, 60100, p));
  pub.queued(21, 60100);
  pub.complete(false, 60200);
  assert(pub.sentValue == 20);
  assert(!pub.due(21, 61000, p));
  assert(pub.due(21, 65100, p));
  pub.reconnect();
  assert(pub.due(20, 65100, p));
  p.periodic = false;
  p.onChange = false;
  assert(!pub.due(20, 999999, p));
  p.onChange = true;
  assert(pub.due(20, 999999, p));
  assert(!pub.due(NAN, 999999, p));
  assert(sensor::elapsed(100, 0xfffffff0u, 116));
  assert(!sensor::elapsed(100, 0xfffffff0u, 117));
  std::cout << "PASS: DPT9 vectors, group/ROM validation, publication ACK/retry/rate policies and "
               "timer wrap\n";
}
