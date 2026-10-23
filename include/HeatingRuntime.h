#pragma once
#include "HeatingCore.h"

namespace heating {
// Room associations are independent of the eight physical manifold positions.
struct RuntimeLayout {
  std::array<int8_t, 8> position{}, zone{};
  RuntimeLayout() {
    for (unsigned i = 0; i < 8; ++i) position[i] = i;
    zone.fill(-1);
  }
  bool valid(unsigned rooms) const {
    bool used[8] = {};
    for (unsigned i = 0; i < 8; ++i) {
      if (zone[i] < -1 || zone[i] >= int(rooms) || position[i] < -1 || position[i] > 7)
        return false;
      if (position[i] >= 0) {
        if (used[position[i]]) return false;
        used[position[i]] = true;
      }
    }
    return true;
  }
};

// KNX reported electrical state plus opening delay, never hydraulic proof.
// The adapter marks commands sent/acknowledged and supplies subsequent feedback.
struct Runtime {
  static constexpr unsigned Pump = 8;
  struct Actuator {
    bool command = false, known = false, reported = false;
    bool sent = false, acknowledged = false;
    uint32_t changed = 0, at = 0;
  };
  std::array<Actuator, 9> output{};
  std::array<bool, 8> demand{};
  bool armed = false, fault = false, connected = false;
  uint32_t openMs = 120000, timeoutMs = 30000;

  void set(unsigned i, bool on, uint32_t now) {
    auto &a = output[i];
    if (a.command != on) {
      a.command = on;
      a.sent = a.acknowledged = a.known = false;
      a.changed = now;
    }
  }
  void report(unsigned i, bool on, uint32_t now) {
    auto &a = output[i];
    if (a.sent) {
      a.known = true;
      a.reported = on;
      a.at = now;
    }
  }
  // Feedback can precede the tunnel ACK. Keep it, but do not confirm the
  // electrical state until both have arrived for the current command.
  // Retain unchanged electrical states within a connection; changes and network
  // loss invalidate them. This does not require periodic bus polling.
  void acknowledge(unsigned i, bool ok) {
    auto &a=output[i];a.acknowledged=ok;
    if(!ok)a.known=false;
  }
  bool confirmed(unsigned i, bool on, uint32_t /*now*/) const {
    const auto &a = output[i];
    return a.sent && a.acknowledged && a.known && a.reported == on;
  }
  void stop(uint32_t now) {
    armed = false;
    demand.fill(false);
    set(Pump, false, now);
  }
  void trip(uint32_t now) { fault = true; stop(now); }
  bool arm(uint32_t now) {
    if (armed || !connected || !confirmed(Pump, false, now)) return false;
    fault = false;
    armed = true;
    demand.fill(false);
    return true;
  }
  void tick(uint32_t now, bool link) {
    if (link != connected) {
      connected = link;
      for (auto &a : output) {
        a.sent = a.acknowledged = a.known = false;
        a.changed = now;
      }
      if (!link) trip(now);
    }
    if (!link) return;
    if (!armed) {
      set(Pump, false, now);
      // Preserve existing paths until a fresh pump OFF report arrives.
      if (confirmed(Pump, false, now))
        for (unsigned i = 0; i < 8; ++i) set(i, false, now);
      return;
    }
    bool changed = false, any = false, ready = true;
    for (unsigned i = 0; i < 8; ++i) {
      changed |= demand[i] != output[i].command;
      any |= demand[i];
    }
    if (output[Pump].command) {
      bool path = false;
      for (unsigned i = 0; i < 8; ++i)
        path |= output[i].command && confirmed(i, true, now);
      if (!path) { trip(now); return; }
      if (changed || !any) set(Pump, false, now);
    }
    if (!output[Pump].command) {
      if (!confirmed(Pump, false, now)) {
        if (output[Pump].sent && age(now, output[Pump].changed) >= timeoutMs)
          trip(now);
        return;
      }
      for (unsigned i = 0; i < 8; ++i) {
        set(i, demand[i], now);
        if (demand[i])
          ready &= confirmed(i, true, now) && age(now, output[i].changed) >= openMs;
      }
      if (any && ready) set(Pump, true, now);
    }
    for (unsigned i = 0; i < 9; ++i) {
      if (output[i].sent && !confirmed(i, output[i].command, now) &&
          age(now, output[i].changed) >= timeoutMs) {
        trip(now);
        return;
      }
    }
  }
};
} // namespace heating
