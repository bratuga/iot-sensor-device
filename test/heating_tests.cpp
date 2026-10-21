#include "HeatingCore.h"
#include <cassert>
#include <iostream>
#include <limits>
using namespace heating;
static Timing timing() { return {10, 100, 50, 1000}; } // test clocks, not installation values
static void feedback(Controller &c, unsigned i, bool on, uint32_t now) {
  assert(c.observe(i, on, Evidence::Physical, c.outputs()[i].revision, now));
}
static void boot(Controller &c, uint32_t now = 0) {
  assert(c.commission(now));
  assert(!c.arm(now, true, true));
  for (unsigned i = 0; i < Circuits; ++i) assert(!c.outputs()[i].valid);
  feedback(c, Pump, false, now);
  c.tick(now, true, true);
  assert(c.arm(now, true, true));
}
static void start(Controller &c, uint32_t now = 0) {
  boot(c, now);
  ManualRequest r; r.circuit[0] = true; r.pumpPermit = true;
  assert(c.manual(r, Source::Web, c.revision(), now) == Result::Accepted);
  c.tick(now, true, true);
  assert(c.phase() == Phase::Opening && !c.outputs()[Pump].on);
  feedback(c, 0, true, now);
  c.tick(now + 9, true, true); assert(!c.outputs()[Pump].on);
  c.tick(now + 10, true, true); assert(c.outputs()[Pump].on);
  feedback(c, Pump, true, now + 10);
}
int main() {
  { Controller c; assert(!c.commission(0)); assert(c.phase() == Phase::Unconfigured); }
  { Controller c(timing()); boot(c);
    ManualRequest r; r.pumpPermit = true;
    assert(c.manual(r, Source::Knx, c.revision(), 0) == Result::Accepted);
    c.tick(1, true, true); assert(!c.outputs()[Pump].on);
    r.circuit[6] = true;
    assert(c.manual(r, Source::Web, c.revision() - 1, 1) == Result::Conflict);
    assert(!c.requested().circuit[6]);
  }
  { Controller c(timing()); boot(c); ManualRequest r; r.circuit.fill(true); r.pumpPermit = true;
    assert(c.manual(r, Source::Knx, c.revision(), 0) == Result::Accepted); c.tick(0, true, true);
    for (unsigned i = 0; i < Circuits; ++i) feedback(c, i, true, 1);
    c.tick(10, true, true); assert(c.outputs()[Pump].on);
  }
  { Controller c(timing()); start(c); ManualRequest r;
    c.manual(r, Source::Knx, c.revision(), 11); c.tick(11, true, true);
    assert(!c.outputs()[Pump].on && c.outputs()[0].on && c.phase() == Phase::Stopping);
    assert(!c.observe(Pump, false, Evidence::TunnelAck, c.outputs()[Pump].revision, 12));
    assert(!c.observe(Pump, false, Evidence::Commanded, c.outputs()[Pump].revision, 12));
    assert(!c.observe(Pump, false, Evidence::Physical, c.outputs()[Pump].revision - 1, 12));
    c.tick(12, true, true); assert(c.outputs()[0].on);
    feedback(c, Pump, false, 13); c.tick(13, true, true);
    assert(!c.outputs()[0].on && c.phase() == Phase::Idle);
  }
  { Controller c(timing()); start(c); c.tick(11, true, false);
    assert(c.fault() == Fault::Transport && !c.outputs()[Pump].on && c.outputs()[0].on);
    assert(!c.arm(12, true, true));
    ManualRequest r; assert(c.manual(r, Source::Web, c.revision(), 12) == Result::Inhibited);
    feedback(c, Pump, false, 13); c.tick(13, true, true);
    assert(!c.outputs()[0].on && c.phase() == Phase::Fault);
    assert(c.arm(14, true, true)); c.tick(14, true, true); assert(!c.outputs()[Pump].on);
  }
  { Controller c(timing()); start(c); feedback(c, Pump, false, 11); c.tick(11, true, true);
    assert(c.fault() == Fault::Feedback && !c.outputs()[Pump].on && c.outputs()[0].on);
  }
  { Controller c(timing()); start(c); c.tick(111, true, true);
    assert(c.fault() == Fault::Feedback && c.outputs()[0].on && !c.outputs()[Pump].on);
  }
  { Controller c(timing()); boot(c); ManualRequest r; r.circuit[2] = true; r.pumpPermit = true;
    c.manual(r, Source::Web, c.revision(), 0); c.tick(0, true, true); c.tick(50, true, true);
    assert(c.fault() == Fault::Timeout && !c.outputs()[Pump].on);
  }
  { Controller c(timing()); start(c); ManualRequest r;
    c.manual(r, Source::Web, c.revision(), 11); c.tick(11, true, true); c.tick(61, true, true);
    assert(c.fault() == Fault::Timeout && c.outputs()[0].on);
  }
  { Controller c(timing()); start(c); c.tick(11, false, true);
    assert(c.fault() == Fault::Interlock && !c.outputs()[Pump].on);
  }
  { Controller c(timing()); start(c); c.trip(Fault::Maintenance, 11);
    assert(c.fault() == Fault::Maintenance && !c.outputs()[Pump].on && c.outputs()[0].on);
  }
  { Controller c(timing()); start(c); c.tick(1000, true, true);
    assert(c.fault() == Fault::Lease && !c.outputs()[Pump].on);
  }
  { Controller c(timing()); start(c, 0xfffffff8u); assert(c.outputs()[Pump].on);
    c.tick(3, true, true); assert(c.phase() == Phase::Running);
  }
  // Exhaustive manual demand masks: never run with no path or without permission.
  for (unsigned mask = 0; mask < 128; ++mask) {
    for (bool permit : {false, true}) {
      Controller c(timing()); boot(c); ManualRequest r; r.pumpPermit = permit;
      for (unsigned i = 0; i < Circuits; ++i) r.circuit[i] = (mask & (1u << i)) != 0;
      c.manual(r, Source::Web, c.revision(), 0); c.tick(0, true, true);
      for (unsigned i = 0; i < Circuits; ++i) feedback(c, i, r.circuit[i], 1);
      c.tick(10, true, true); assert(c.outputs()[Pump].on == (permit && mask != 0));
      if (c.outputs()[Pump].on) {
        feedback(c, Pump, true, 10);
        ManualRequest next; next.circuit[6] = true; next.pumpPermit = true;
        c.manual(next, Source::Knx, c.revision(), 11); c.tick(11, true, true);
        if (mask != 64) {
          assert(!c.outputs()[Pump].on && c.phase() == Phase::Stopping);
          // New demand cannot close old paths before pump OFF proof.
          for (unsigned i = 0; i < Circuits; ++i) assert(c.outputs()[i].on == r.circuit[i]);
          feedback(c, Pump, false, 12); c.tick(12, true, true);
          feedback(c, 6, true, 12); c.tick(22, true, true);
          assert(c.outputs()[Pump].on && c.outputs()[6].on);
          for (unsigned i = 0; i < 6; ++i) assert(!c.outputs()[i].on);
        }
      }
    }
  }
  { std::array<Binding, Outputs> b{}; assert(!validBindings(b, 4));
    for (unsigned i = 0; i < Outputs; ++i) { b[i].backend = Backend::Local; b[i].localChannel = int(i); }
    assert(!validBindings(b, 4)); assert(validBindings(b, 8));
    b[7].localChannel = 0; assert(!validBindings(b, 8));
    // Synthetic test identifiers only, never deployment defaults.
    b[7] = {Backend::Knx, -1, 100, 101}; assert(validBindings(b, 7));
    b[6] = {Backend::Knx, -1, 101, 102}; assert(!validBindings(b, 6));
    b[6] = {Backend::Knx, -1, 102, 103}; assert(validBindings(b, 6));
    uint16_t reserved[] = {103}; assert(!validBindings(b, 6, reserved, 1));
  }
  { assert(ZoneForCircuit[0] == ZoneForCircuit[1]); assert(ZoneForCircuit[2] == ZoneForCircuit[3]);
    ZoneInput z; assert(!z.demand(0, false));
    z.source = TemperatureSource::Ds18b20; z.valid = z.setpointValid = true;
    z.temperature = 19; z.setpoint = 20; z.hysteresis = 1; z.maxAgeMs = 100;
    assert(z.demand(0, false)); z.temperature = 20; assert(!z.demand(0, false));
    assert(z.demand(0, true)); z.temperature = 20.5; assert(!z.demand(0, true));
    z.temperature = 19; assert(!z.demand(100, true));
    z.temperature = std::numeric_limits<float>::quiet_NaN(); assert(!z.fresh(1));
    z.temperature = 19; z.receivedAt = 0xfffffff8u; assert(z.fresh(2));
  }
  std::cout << "Heating core tests passed\n";
}
