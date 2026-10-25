#pragma once
#include <cmath>

namespace thermostat {
inline bool demand(bool enabled, bool sensorFresh, bool previous,
                   float temperature, float setpoint, float margin) {
  if (!enabled || !sensorFresh || !std::isfinite(temperature)) return false;
  if (temperature <= setpoint - margin) return true;
  if (temperature >= setpoint + margin) return false;
  return previous;
}
}
