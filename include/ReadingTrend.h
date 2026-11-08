#pragma once
#include <cmath>
#include <cstdint>

namespace sensor {
class ReadingTrend {
  static constexpr uint32_t Minute = 60000;
  static constexpr uint32_t Window = 30 * Minute;
  struct Sample { uint32_t at; float value; };
  Sample samples_[32]{};
  unsigned next_ = 0, count_ = 0;

public:
  void sample(uint32_t now, float value, bool valid) {
    if (!valid || !std::isfinite(value)) { next_ = count_ = 0; return; }
    if (count_ && uint32_t(now - samples_[(next_ + 31) % 32].at) < Minute) return;
    samples_[next_] = {now, value};
    next_ = (next_ + 1) % 32;
    if (count_ < 32) ++count_;
  }

  bool delta(uint32_t now, float value, float &result) const {
    for (unsigned i = 0; i < count_; ++i) {
      const auto &s = samples_[(next_ + 31 - i) % 32];
      const uint32_t age = now - s.at;
      if (age < Window) continue;
      if (age > Window + 2 * Minute) return false;
      result = value - s.value;
      return true;
    }
    return false;
  }
};
} // namespace sensor
