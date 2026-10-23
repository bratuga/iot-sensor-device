#pragma once
#include "HeatingCore.h"
namespace heating {
// Persist measurements for display and stable setpoints, never output states or
// a millis() timestamp from an earlier boot. Fresh temperature must come from KNX.
struct Snapshot {
  uint16_t temperatureGroup = 0, setpointGroup = 0;
  float temperature = 0, setpoint = 0;
  bool hasTemperature = false, hasSetpoint = false;
  void restore(ZoneInput &input) const {
    if (hasTemperature && std::isfinite(temperature) && temperature >= -40 && temperature <= 100)
      input.temperature = temperature;
    input.valid = false;
    input.receivedAt = 0;
    input.setpointValid = hasSetpoint && std::isfinite(setpoint) && setpoint >= 5 && setpoint <= 35;
    if (input.setpointValid) input.setpoint = setpoint;
  }
};
}
