#include "ConfigJournal.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>

int main() {
  using namespace configjournal;
  assert(crc32("123456789", 9) == 0xcbf43926UL);
  const std::string old = R"({"oneWire":-1,"name":"existing"})";
  const std::string updated = R"({"oneWire":27,"name":"existing"})";
  Header first{Magic, 41, uint32_t(old.size()), crc32(old.data(), old.size())};
  Header second{Magic, 42, uint32_t(updated.size()), crc32(updated.data(), updated.size())};
  assert(valid(first, sizeof(first) + old.size(), old.data()));
  assert(valid(second, sizeof(second) + updated.size(), updated.data()));
  assert(newest(true, first.sequence, true, second.sequence) == 1);
  // Power loss during a write leaves the other slot intact and selectable.
  assert(!valid(second, sizeof(second) + updated.size() - 1, updated.data()));
  assert(newest(true, first.sequence, false, second.sequence) == 0);
  std::string damaged = updated; damaged[12] = '9';
  assert(!valid(second, sizeof(second) + damaged.size(), damaged.data()));
  assert(!valid(second, sizeof(second) + updated.size(), old.data()));
  assert(newest(false, 0, true, 1) == 1);
  assert(newest(true, 0xffffffffUL, true, 0) == 1);
  std::cout << "PASS: configuration CRC, two-slot updates, interrupted/corrupt write and sequence wrap\n";
}
