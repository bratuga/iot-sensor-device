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
  bool active(unsigned i) const { return i < 8 && zone[i] >= 0; }
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
  enum class Failure { None, LinkLost, Transport, FeedbackTimeout, PathLost, UnexpectedPump };
  Failure failure = Failure::None;
  int faultOutput = -1;
  std::array<bool, 8> enabled{{true,true,true,true,true,true,true,true}};
  bool active(unsigned i) const { return i == Pump || (i < Pump && enabled[i]); }
  void configure(const RuntimeLayout &layout) {
    for(unsigned i=0;i<Pump;++i) enabled[i]=layout.active(i);
  }
  struct Actuator {
    bool command = false, known = false, reported = false;
    bool sent = false, acknowledged = false, feedbackForCommand = false;
    uint32_t changed = 0, at = 0, onSince = 0;
  };
  std::array<Actuator, 9> output{};
  std::array<bool, 8> demand{};
  std::array<bool, 8> external{};
  bool armed = false, fault = false, connected = false, unexpectedPump = false;
  uint32_t openMs = 120000, timeoutMs = 30000;
  uint32_t serviceMs = 900000, pauseMs = 300000;
  uint32_t serviceAt = 0, pauseAt = 0;
  bool serviceActive = false, pauseActive = false;
  enum class PumpPhase { Idle, Waiting, Service, Pause };
  PumpPhase pumpPhase(uint32_t /*now*/) const {
    if (!armed) return PumpPhase::Idle;
    if (pauseActive) return PumpPhase::Pause;
    if (serviceActive) return PumpPhase::Service;
    for (unsigned i=0;i<8;++i) if (active(i) && (demand[i] || external[i]))
      return PumpPhase::Waiting;
    return PumpPhase::Idle;
  }
  uint32_t phaseRemaining(uint32_t now) const {
    if (pauseActive) return !pauseAt || age(now,pauseAt)<pauseMs ?
      (!pauseAt ? pauseMs : pauseMs-age(now,pauseAt)) : 0;
    if (serviceActive) return age(now,serviceAt)>=serviceMs ? 0 : serviceMs-age(now,serviceAt);
    if (pumpPhase(now)==PumpPhase::Waiting) {
      uint32_t remaining=openMs;
      for(unsigned i=0;i<8;++i)if(active(i)&&(demand[i]||external[i])&&pathConfirmed(i,now)){
        uint32_t elapsed=age(now,output[i].onSince);
        if(elapsed>=openMs)return 0;
        if(openMs-elapsed<remaining)remaining=openMs-elapsed;
      }
      return remaining;
    }
    return 0;
  }
  bool pathConfirmed(unsigned i,uint32_t now) const {
    return active(i) && output[i].known && output[i].reported &&
      (confirmed(i,true,now) || (external[i] && !demand[i]));
  }
  bool pathReady(unsigned i,uint32_t now) const {
    return pathConfirmed(i,now) && age(now,output[i].onSince)>=openMs;
  }

  // Shared by firmware and simulator: one command at a time; unused ports
  // never generate traffic. Never remove the last open path under a running pump.
  int nextCommand(uint32_t now) const {
    for(unsigned k=0;k<9;++k){unsigned i=(k+Pump)%9;
      if(!active(i) || output[i].sent) continue;
      if(i!=Pump && !output[Pump].command && !confirmed(Pump,false,now)) continue;
      if(i!=Pump && !output[i].command && output[Pump].command){
        bool replacement=false;
        for(unsigned j=0;j<Pump;++j)if(j!=i && (demand[j]||external[j]) && pathReady(j,now))replacement=true;
        if(!replacement)continue;
      }
      return int(i);
    }
    return -1;
  }
  void set(unsigned i, bool on, uint32_t now) {
    if (!active(i)) return;
    auto &a = output[i];
    if (a.command != on) {
      a.command = on;
      a.sent = a.acknowledged = a.feedbackForCommand = false;
      a.changed = now;
      if(i<Pump && on && demand[i]) external[i]=false;
    }
  }
  void report(unsigned i, bool on, uint32_t now) {
    if (!active(i)) return;
    auto &a = output[i];
    const bool unsolicited = a.known && a.reported != on &&
      ((a.sent && a.acknowledged && a.feedbackForCommand) ||
       (!a.sent && a.command == a.reported));
    if(i<Pump && unsolicited) external[i]=on;
    if(i<Pump && !on) external[i]=false;
    if(i<Pump && a.sent && a.command==on && demand[i]) external[i]=false;
    if(i==Pump && on && !a.command) {
      bool safe=armed && !pauseActive;
      bool path=false;
      for(unsigned j=0;j<Pump;++j)path |= pathReady(j,now);
      if(!safe || !path) unexpectedPump=true;
    }
    if(on && (!a.known || !a.reported)) a.onSince=now;
    a.known = true;
    a.reported = on;
    a.at = now;
    if(a.sent) a.feedbackForCommand = a.command == on;
    if(i==Pump && unsolicited && !unexpectedPump){
      a.command=on;a.sent=a.acknowledged=a.feedbackForCommand=true;
      a.changed=now;
      serviceActive=on;serviceAt=now;
      if(!on && armed){pauseActive=true;pauseAt=now;}
    }
  }
  // Feedback can precede the tunnel ACK. Keep it, but do not confirm the
  // electrical state until both have arrived for the current command.
  // Retain unchanged electrical states within a connection; changes and network
  // loss invalidate them. This does not require periodic bus polling.
  void acknowledge(unsigned i, bool ok) {
    auto &a=output[i];a.acknowledged=ok;
    if(!ok)a.feedbackForCommand=false;
  }
  bool confirmed(unsigned i, bool on, uint32_t /*now*/) const {
    const auto &a = output[i];
    return active(i) && a.command == on && a.sent && a.acknowledged &&
      a.feedbackForCommand && a.known && a.reported == on;
  }
  void stop(uint32_t now) {
    armed = false;
    serviceActive = pauseActive = false;serviceAt=pauseAt=0;
    demand.fill(false);
    set(Pump, false, now);
  }
  void trip(uint32_t now, Failure why=Failure::Transport, int index=-1) {
    if(!fault){ failure=why; faultOutput=index; }
    fault=true; stop(now);
  }
  bool timedOut(unsigned i, uint32_t now) const {
    const auto &a=output[i];
    return active(i) && a.sent && !(i<Pump && external[i]) &&
      !confirmed(i,a.command,now) && age(now,a.changed)>=timeoutMs;
  }
  bool canArm(uint32_t now) const {
    if(armed || !connected || !confirmed(Pump,false,now)) return false;
    for(unsigned i=0;i<Pump;++i) if(active(i) && !confirmed(i,false,now)) return false;
    return true;
  }
  bool arm(uint32_t now) {
    if (!canArm(now)) return false;
    fault = false; failure=Failure::None; faultOutput=-1;
    armed = true;
    serviceActive = pauseActive = false;serviceAt=pauseAt=0;
    demand.fill(false);
    return true;
  }
  void tick(uint32_t now, bool link) {
    if (link != connected) {
      connected = link; unexpectedPump=false;
      for (auto &a : output) {
        a.sent = a.acknowledged = a.feedbackForCommand = a.known = false;
        a.changed = now;
      }
      if (!link) trip(now,Failure::LinkLost);
    }
    if (!link) return;
    // An external ON while OFF is requested must result in a real OFF command.
    // Never close paths on a fabricated OFF observation.
    if(unexpectedPump){
      unexpectedPump=false;
      trip(now,Failure::UnexpectedPump,Pump);
      output[Pump].sent=output[Pump].acknowledged=output[Pump].feedbackForCommand=false;
      output[Pump].changed=now;
    }
    if (!armed) {
      set(Pump, false, now);
      // Preserve existing paths until a fresh pump OFF report arrives.
      if (confirmed(Pump, false, now))
        for (unsigned i = 0; i < 8; ++i) set(i, false, now);
      for(unsigned i=0;i<9;++i) if(timedOut(i,now)) {
        trip(now,Failure::FeedbackTimeout,int(i)); break;
      }
      return;
    }
    bool any = false, ready = true;
    for (unsigned i = 0; i < 8; ++i) {
      if(!active(i)) continue;
      any |= demand[i] || external[i];
    }
    if(!any){pauseActive=false;pauseAt=0;}
    if (output[Pump].command) {
      bool path = false;
      for (unsigned i = 0; i < 8; ++i)
        path |= active(i) && output[i].known && output[i].reported;
      if (!path && any) { trip(now,Failure::PathLost); return; }
      if(confirmed(Pump,true,now) && !serviceActive){serviceActive=true;serviceAt=now;}
      // Opening new valves never removes an existing hydraulic path.
      for (unsigned i = 0; i < 8; ++i)
        if (active(i) && demand[i]) set(i, true, now);
      bool retained=false;
      for(unsigned i=0;i<8;++i)if(active(i)&&(demand[i]||external[i])&&pathReady(i,now))retained=true;
      if(retained)for(unsigned i=0;i<8;++i)
        if(active(i)&&!demand[i]&&!external[i])set(i,false,now);
      if(!any || (serviceActive && age(now,serviceAt)>=serviceMs)) {
        if(any && serviceActive) pauseActive=true;
        serviceActive=false;
        set(Pump,false,now);
      }
    }
    if (!output[Pump].command) {
      if (!confirmed(Pump, false, now)) {
        if (timedOut(Pump,now))
          trip(now,Failure::FeedbackTimeout,Pump);
        return;
      }
      for (unsigned i = 0; i < 8; ++i) {
        if(!active(i)) continue;
        if(demand[i] || !external[i])set(i,demand[i],now);
        // Finish OFF transitions too, before allowing the pump to restart.
        if(demand[i] || external[i]) ready &= pathReady(i,now);
        else ready &= confirmed(i,false,now);
      }
      if(pauseActive){
        if(!confirmed(Pump,false,now))return;
        if(pauseAt==0)pauseAt=now;
        if(age(now,pauseAt)>=pauseMs){pauseActive=false;pauseAt=0;}
      }
      if (any && ready && !pauseActive) set(Pump, true, now);
    }
    for (unsigned i = 0; i < 9; ++i) {
      if (timedOut(i,now)) {
        trip(now,Failure::FeedbackTimeout,int(i));
        return;
      }
    }
  }
};
} // namespace heating
