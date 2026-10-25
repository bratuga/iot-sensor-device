#pragma once
#include <algorithm>
#include <cstdint>
inline void delay(unsigned) {}
template <class T> T constrain(T value, T lo, T hi) {
  return std::max(lo, std::min(value, hi));
}
