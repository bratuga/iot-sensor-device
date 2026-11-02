#pragma once
#include "HeatingRuntime.h"
#include <cmath>
#include <cstdint>

namespace heating {

enum class ThermalPauseReason : uint8_t { None, ColdSupply, HotReturn };

struct ThermalGuardOptions {
  bool coldSupplyEnabled = true;
  float coldSupplyLimitC = 30.0f;
  uint32_t coldSupplyBelowMs = 300000;
  uint32_t coldSupplyPauseMs = 900000;
  bool hotReturnEnabled = true;
  float hotReturnLimitC = 28.0f;
  uint32_t hotReturnAboveMs = 300000;
  uint32_t hotReturnPauseMs = 900000;
};

// The guard masks all heat requests for a timed pause while retaining the
// user's armed state outside this object. The caller owns the safe OFF sequence
// and may re-arm only after OFF feedback has been confirmed.
class PumpThermalGuard {
  ThermalPauseReason reason_ = ThermalPauseReason::None;
  uint32_t pauseAt_ = 0, pauseMs_ = 0, returnAboveAt_ = 0, supplyBelowAt_ = 0;
  bool timingReturn_ = false, timingSupply_ = false;

  void pause(ThermalPauseReason reason, uint32_t now, uint32_t duration) {
    reason_ = reason;
    pauseAt_ = now;
    pauseMs_ = duration;
    timingReturn_ = false;
    timingSupply_ = false;
  }

public:
  bool update(uint32_t now, bool pumpRunning,
              bool supplyValid, float supplyC,
              bool returnValid, float returnC,
              const ThermalGuardOptions &options) {
    if (reason_ != ThermalPauseReason::None) {
      if (uint32_t(now - pauseAt_) < pauseMs_) return true;
      reason_ = ThermalPauseReason::None;
      pauseAt_ = pauseMs_ = 0;
      timingReturn_ = false;
      timingSupply_ = false;
    }

    if (!pumpRunning) {
      timingReturn_ = false;
      timingSupply_ = false;
      return false;
    }

    if (options.coldSupplyEnabled && supplyValid && std::isfinite(supplyC) &&
        supplyC < options.coldSupplyLimitC) {
      if (!timingSupply_) {
        timingSupply_ = true;
        supplyBelowAt_ = now;
      }
      if (uint32_t(now - supplyBelowAt_) >= options.coldSupplyBelowMs) {
        pause(ThermalPauseReason::ColdSupply, now, options.coldSupplyPauseMs);
        return true;
      }
    } else timingSupply_ = false;

    if (!options.hotReturnEnabled || !returnValid || !std::isfinite(returnC)) {
      timingReturn_ = false;
      return false;
    }
    if (returnC <= options.hotReturnLimitC) {
      timingReturn_ = false;
      return false;
    }
    if (!timingReturn_) {
      timingReturn_ = true;
      returnAboveAt_ = now;
      return false;
    }
    if (uint32_t(now - returnAboveAt_) >= options.hotReturnAboveMs) {
      pause(ThermalPauseReason::HotReturn, now, options.hotReturnPauseMs);
      return true;
    }
    return false;
  }

  void reset() {
    reason_ = ThermalPauseReason::None;
    pauseAt_ = pauseMs_ = returnAboveAt_ = supplyBelowAt_ = 0;
    timingReturn_ = timingSupply_ = false;
  }

  bool paused() const { return reason_ != ThermalPauseReason::None; }
  ThermalPauseReason reason() const { return reason_; }
  uint32_t remainingMs(uint32_t now) const {
    if (!paused()) return 0;
    const uint32_t elapsed = uint32_t(now - pauseAt_);
    return elapsed >= pauseMs_ ? 0 : pauseMs_ - elapsed;
  }
  uint32_t durationMs() const { return pauseMs_; }
};

// Couples the thermal timer to the KNX output sequencer. The runtime owns
// the safe order (pump OFF feedback, then valves OFF); this supervisor only
// pauses demand and re-arms after the timed pause and confirmed OFF states.
class PumpThermalSupervisor {
  PumpThermalGuard guard_;
  bool resumeArmed_ = false;

public:
  bool update(Runtime &runtime, uint32_t now,
              bool supplyValid, float supplyC,
              bool returnValid, float returnC,
              const ThermalGuardOptions &options) {
    const auto &pump = runtime.output[Runtime::Pump];
    // Count actual circulation only after KNX feedback confirms the pump ON.
    const bool pumpRunning = pump.known && pump.reported;
    const bool paused = guard_.update(now, pumpRunning, supplyValid, supplyC,
                                      returnValid, returnC, options);
    if (paused && !resumeArmed_ && runtime.armed) {
      resumeArmed_ = true;
      runtime.stop(now);
    }
    if (runtime.fault) resumeArmed_ = false;
    if (!paused && resumeArmed_ && runtime.arm(now)) resumeArmed_ = false;
    return paused;
  }

  void cancelResume() { resumeArmed_ = false; }
  void reset() { guard_.reset(); resumeArmed_ = false; }
  bool paused() const { return guard_.paused(); }
  bool resumeArmed() const { return resumeArmed_; }
  ThermalPauseReason reason() const { return guard_.reason(); }
  uint32_t remainingMs(uint32_t now) const { return guard_.remainingMs(now); }
  uint32_t durationMs() const { return guard_.durationMs(); }
};

} // namespace heating
