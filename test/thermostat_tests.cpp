#include "ThermostatCore.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main() {
  assert(thermostat::demand(true,true,false,20.0f,21.0f,0.3f));
  assert(thermostat::demand(true,true,true,20.9f,21.0f,0.3f));
  assert(!thermostat::demand(true,true,true,21.4f,21.0f,0.3f));
  assert(!thermostat::demand(false,true,true,20.0f,21.0f,0.3f));
  assert(!thermostat::demand(true,false,true,20.0f,21.0f,0.3f));
  assert(!thermostat::demand(true,true,true,NAN,21.0f,0.3f));
  std::cout << "PASS thermostat: hysteresis, OFF and stale sensor fail-safe\n";
}
