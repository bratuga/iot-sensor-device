#pragma once
#include <array>
#include <cmath>
#include <cstdint>

// Portable planning core: no GPIO, network, Arduino or persistent ON state.
namespace heating {
constexpr unsigned Circuits = 7, Zones = 5, Pump = 7, Outputs = 8;
constexpr std::array<unsigned, Circuits> ZoneForCircuit{{0, 0, 1, 1, 2, 3, 4}};
// The manifold has eight positions; seven logical circuits may be assigned.
// An unassigned position (-1) never implies a live actuator.
struct Layout {
  std::array<int8_t, Circuits> position{};
  std::array<uint8_t, Circuits> zone{{0, 0, 1, 1, 2, 3, 4}};
  Layout() { position.fill(-1); }
  bool valid() const {
    bool used[8] = {};
    for (unsigned i = 0; i < Circuits; ++i) {
      if (zone[i] >= Zones || position[i] < -1 || position[i] >= 8) return false;
      if (position[i] >= 0) {
        if (used[position[i]]) return false;
        used[position[i]] = true;
      }
    }
    return true;
  }
};
inline uint32_t age(uint32_t now, uint32_t then) { return now - then; }
inline bool duration(uint32_t ms) { return ms > 0 && ms < 0x80000000u; }
enum class Backend { Unassigned, Local, Knx };
struct Binding {
  Backend backend = Backend::Unassigned;
  int localChannel = -1;
  uint16_t commandGroup = 0, physicalGroup = 0;
};
// No default addresses. Reserved groups include existing relay and sensor objects.
inline bool validBindings(const std::array<Binding, Outputs> &b, unsigned localCount,
                          const uint16_t *reserved = nullptr, unsigned count = 0) {
  if (count && !reserved) return false;
  for (unsigned i = 0; i < Outputs; ++i) {
    const auto &x = b[i];
    if (x.backend == Backend::Local) {
      if (x.localChannel < 0 || unsigned(x.localChannel) >= localCount || x.commandGroup)
        return false;
    } else if (x.backend == Backend::Knx) {
      if (!x.commandGroup || x.localChannel != -1) return false;
    } else return false;
    // Physical inputs can be provided by a separate local adapter (group = 0).
    if (x.commandGroup && x.commandGroup == x.physicalGroup) return false;
    for (unsigned k = 0; k < count; ++k)
      if (reserved[k] && (reserved[k] == x.commandGroup || reserved[k] == x.physicalGroup))
        return false;
    for (unsigned j = 0; j < i; ++j) {
      const auto &y = b[j];
      if (x.backend == Backend::Local && y.backend == Backend::Local &&
          x.localChannel == y.localChannel) return false;
      if ((x.commandGroup && (x.commandGroup == y.commandGroup || x.commandGroup == y.physicalGroup)) ||
          (x.physicalGroup && (x.physicalGroup == y.commandGroup || x.physicalGroup == y.physicalGroup)))
        return false;
    }
  }
  return true;
}
struct Timing {
  // All zero: commissioning required; these are NOT installation defaults.
  uint32_t openMs = 0, feedbackMs = 0, transitionMs = 0, manualLeaseMs = 0;
  bool valid() const {
    return duration(openMs) && duration(feedbackMs) && duration(transitionMs) &&
           duration(manualLeaseMs) && transitionMs > openMs;
  }
};
enum class Phase { Unconfigured, Disarmed, Idle, Opening, Running, Stopping, Fault };
enum class Fault { None, Interlock, Transport, Feedback, Timeout, Lease, Maintenance };
enum class Source { Web, Knx };
enum class Result { Accepted, Conflict, Inhibited, Invalid };
enum class Evidence { Commanded, TunnelAck, Physical };
struct Output {
  bool valid = false, on = false;
  uint32_t revision = 0, since = 0;
};
struct Observation {
  bool valid = false, on = false;
  uint32_t revision = 0, at = 0;
};
struct ManualRequest {
  std::array<bool, Circuits> circuit{};
  bool pumpPermit = false; // Permission with demand, never an unconditional force ON.
};
class Controller {
 public:
  explicit Controller(Timing timing = {}) : timing_(timing) {}
  const std::array<Output, Outputs> &outputs() const { return out_; }
  const ManualRequest &requested() const { return request_; }
  const std::array<Observation, Outputs> &observations() const { return observed_; }
  Phase phase() const { return phase_; }
  Fault fault() const { return fault_; }
  uint32_t revision() const { return revision_; }
  Source source() const { return source_; }
  // Call only after validating bindings and commissioning the physical evidence path.
  bool commission(uint32_t now) {
    if (phase_ != Phase::Unconfigured || !timing_.valid()) return false;
    phase_ = Phase::Disarmed;
    set(Pump, false, now); // Do not close any valve while pump state is unknown.
    return true;
  }
  bool observe(unsigned i, bool on, Evidence evidence, uint32_t revision, uint32_t now) {
    if (i >= Outputs || evidence != Evidence::Physical || !out_[i].valid ||
        out_[i].revision != revision) return false;
    observed_[i] = {true, on, revision, now};
    return true;
  }
  bool confirmed(unsigned i, bool on, uint32_t now) const {
    if (i >= Outputs) return false;
    const auto &o = observed_[i];
    return o.valid && o.on == on && o.revision == out_[i].revision &&
           age(now, o.at) < timing_.feedbackMs;
  }
  // Health must be provided every tick by the adapter; false trips and latches.
  void tick(uint32_t now, bool interlockOk, bool transportOk) {
    if (phase_ == Phase::Unconfigured) return;
    if (!interlockOk) trip(Fault::Interlock, now);
    else if (!transportOk) trip(Fault::Transport, now);
    if (armed_ && age(now, lastManual_) >= timing_.manualLeaseMs) trip(Fault::Lease, now);
    if (!armed_) {
      set(Pump, false, now);
      if (confirmed(Pump, false, now)) closeAll(now);
      return;
    }
    const bool demand = request_.pumpPermit && any(request_.circuit);
    if (phase_ == Phase::Running) {
      if (confirmed(Pump, false, now)) { trip(Fault::Feedback, now); return; }
      bool path = false;
      for (unsigned i = 0; i < Circuits; ++i)
        path = path || (out_[i].on && confirmed(i, true, now));
      if (!path) { trip(Fault::Feedback, now); return; }
      if (!confirmed(Pump, true, now) && age(now, out_[Pump].since) >= timing_.transitionMs) {
        trip(Fault::Feedback, now); return;
      }
      // Any circuit change stops the pump first: intentionally conservative phase 1.
      bool changed = false;
      for (unsigned i = 0; i < Circuits; ++i) changed |= out_[i].on != request_.circuit[i];
      if (!demand || changed) { set(Pump, false, now); phase_ = Phase::Stopping; }
      else return;
    }
    if (phase_ == Phase::Stopping) {
      if (!confirmed(Pump, false, now)) {
        if (age(now, out_[Pump].since) >= timing_.transitionMs) trip(Fault::Timeout, now);
        return; // Preserve all previously open paths until pump OFF is proven.
      }
      phase_ = Phase::Idle;
    }
    // Also require a fresh OFF observation before changing valves in Idle/Opening.
    if (!confirmed(Pump, false, now)) { trip(Fault::Feedback, now); return; }
    bool ready = false, allRequestedReady = true;
    for (unsigned i = 0; i < Circuits; ++i) {
      set(i, request_.circuit[i], now);
      if (request_.circuit[i]) {
        const bool open = confirmed(i, true, now) && age(now, out_[i].since) >= timing_.openMs;
        ready |= open;
        allRequestedReady &= open;
        if (!open && age(now, out_[i].since) >= timing_.transitionMs) {
          trip(Fault::Timeout, now); return;
        }
      }
    }
    if (demand && ready && allRequestedReady) {
      set(Pump, true, now);
      phase_ = Phase::Running;
    } else phase_ = demand ? Phase::Opening : Phase::Idle;
  }
  bool arm(uint32_t now, bool interlockOk, bool transportOk) {
    if (phase_ == Phase::Unconfigured || armed_ || !interlockOk || !transportOk ||
        !confirmed(Pump, false, now)) return false;
    // Explicit re-arm never restores previous demand.
    request_ = {}; fault_ = Fault::None; armed_ = true; phase_ = Phase::Idle;
    lastManual_ = now; ++revision_; return true;
  }
  Result manual(const ManualRequest &r, Source source, uint32_t expected, uint32_t now) {
    if (source != Source::Web && source != Source::Knx) return Result::Invalid;
    if (!armed_) return Result::Inhibited;
    if (expected != revision_) return Result::Conflict;
    request_ = r; source_ = source; lastManual_ = now; ++revision_;
    return Result::Accepted;
  }
  void trip(Fault reason, uint32_t now) {
    if (phase_ == Phase::Unconfigured) return;
    if (fault_ == Fault::None) fault_ = reason;
    request_ = {}; armed_ = false; phase_ = Phase::Fault;
    set(Pump, false, now); ++revision_;
  }
 private:
  static bool any(const std::array<bool, Circuits> &v) {
    for (bool x : v) if (x) return true;
    return false;
  }
  void set(unsigned i, bool on, uint32_t now) {
    auto &o = out_[i];
    if (!o.valid || o.on != on) {
      o.valid = true; o.on = on; ++o.revision; o.since = now;
      observed_[i].valid = false;
    }
  }
  void closeAll(uint32_t now) { for (unsigned i = 0; i < Circuits; ++i) set(i, false, now); }
  Timing timing_;
  std::array<Output, Outputs> out_{};
  std::array<Observation, Outputs> observed_{};
  ManualRequest request_;
  Phase phase_ = Phase::Unconfigured;
  Fault fault_ = Fault::None;
  Source source_ = Source::Web;
  bool armed_ = false;
  uint32_t revision_ = 0, lastManual_ = 0;
};

// Future thermostat input only: not connected to Controller::manual or outputs.
enum class TemperatureSource { Unassigned, Bme280, Ds18b20, Knx };
struct ZoneInput {
  TemperatureSource source = TemperatureSource::Unassigned;
  float temperature = 0, setpoint = 0, hysteresis = 0;
  uint32_t receivedAt = 0, maxAgeMs = 0;
  bool valid = false, setpointValid = false;
  bool fresh(uint32_t now) const {
    return source != TemperatureSource::Unassigned && valid && setpointValid &&
           std::isfinite(temperature) && std::isfinite(setpoint) &&
           std::isfinite(hysteresis) && hysteresis > 0 && duration(maxAgeMs) &&
           age(now, receivedAt) < maxAgeMs;
  }
  bool demand(uint32_t now, bool previous) const {
    if (!fresh(now)) return false;
    return previous ? temperature < setpoint + hysteresis / 2
                    : temperature <= setpoint - hysteresis / 2;
  }
};
} // namespace heating
