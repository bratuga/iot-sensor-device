#include "PumpThermalGuard.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
using namespace heating;

static void feedback(Runtime &runtime, unsigned index, bool state, uint32_t now) {
  runtime.output[index].sent=true;
  runtime.acknowledge(index,true);
  runtime.report(index,state,now);
  assert(runtime.confirmed(index,state,now));
}

int main() {
  ThermalGuardOptions o;
  o.coldSupplyPauseMs=900000;
  o.hotReturnAboveMs=300000;
  o.hotReturnPauseMs=900000;

  PumpThermalGuard guard;
  assert(!guard.update(100,false,true,20,true,40,o));
  assert(!guard.paused());
  assert(!guard.update(200,true,true,30,true,28,o));
  assert(guard.reason()==ThermalPauseReason::None);

  assert(!guard.update(300,true,true,29.9f,false,NAN,o));
  assert(!guard.update(300299,true,true,29.9f,false,NAN,o));
  assert(guard.update(300300,true,true,29.9f,false,NAN,o));
  assert(guard.reason()==ThermalPauseReason::ColdSupply);
  assert(guard.remainingMs(300300)==900000);
  assert(guard.update(750300,false,true,35,true,20,o));
  assert(guard.remainingMs(750300)==450000);
  assert(!guard.update(1200300,false,true,20,true,20,o));
  assert(!guard.paused());
  // Every retry gets a fresh five-minute circulation window, even when cold.
  assert(!guard.update(1200400,true,true,20,false,NAN,o));
  assert(!guard.update(1500399,true,true,20,false,NAN,o));
  assert(guard.update(1500400,true,true,20,false,NAN,o));
  assert(!guard.update(2400400,false,true,20,false,NAN,o));
  assert(!guard.update(2400500,true,true,20,false,NAN,o));
  // Reaching the limit clears the cold timer and operation continues normally.
  assert(!guard.update(2700499,true,true,30,false,NAN,o));
  assert(!guard.update(2700500,true,true,29,false,NAN,o));
  assert(!guard.update(3000499,true,true,29,false,NAN,o));
  assert(guard.update(3000500,true,true,29,false,NAN,o));
  guard.reset();
  assert(!guard.update(0xfffffff0u,true,true,20,false,NAN,o));
  assert(guard.update(uint32_t(0xfffffff0u+300000u),true,true,20,false,NAN,o));

  guard.reset();
  assert(!guard.update(1000,true,true,35,true,28.1f,o));
  assert(!guard.update(300999,true,true,35,true,29.0f,o));
  assert(guard.update(301000,true,true,35,true,28.01f,o));
  assert(guard.reason()==ThermalPauseReason::HotReturn);
  assert(guard.remainingMs(301000)==900000);
  assert(!guard.update(1201000,true,true,35,true,27,o));
  assert(guard.reason()==ThermalPauseReason::None);

  guard.reset();
  assert(!guard.update(2000,true,false,NAN,false,NAN,o));
  assert(!guard.update(100000,false,true,31,true,29,o)); // pump-off gap resets the dwell
  assert(!guard.update(399999,true,true,31,true,29,o));
  assert(!guard.update(699998,true,true,31,true,29,o));
  assert(guard.update(699999,true,true,31,true,29,o));

  guard.reset();
  const uint32_t start=0xfffffff0u;
  assert(!guard.update(start,true,true,31,true,29,o));
  assert(guard.update(uint32_t(start+300000u),true,true,31,true,29,o));
  assert(guard.remainingMs(uint32_t(start+300000u))==900000);

  Runtime runtime;
  RuntimeLayout layout;layout.zone[0]=0;runtime.configure(layout);runtime.openMs=0;runtime.connected=true;
  for(unsigned i=0;i<9;++i)if(runtime.active(i)){
    auto &out=runtime.output[i];out.command=false;out.known=true;out.reported=false;
    out.sent=out.acknowledged=out.feedbackForCommand=true;
  }
  assert(runtime.arm(1));runtime.demand[0]=true;runtime.tick(2,true);
  feedback(runtime,0,true,3);runtime.tick(4,true);assert(runtime.output[Runtime::Pump].command);
  feedback(runtime,Runtime::Pump,true,5);runtime.tick(5,true);assert(runtime.armed);

  PumpThermalSupervisor supervisor;
  assert(!supervisor.update(runtime,6,true,29.9f,false,NAN,o));
  assert(!supervisor.update(runtime,300005,true,29.9f,false,NAN,o));
  assert(supervisor.update(runtime,300006,true,29.9f,false,NAN,o));
  assert(supervisor.reason()==ThermalPauseReason::ColdSupply&&supervisor.resumeArmed());
  assert(!runtime.armed&&!runtime.output[Runtime::Pump].command);
  runtime.tick(300006,true);assert(runtime.output[0].command); // Keep the path until pump OFF feedback.
  feedback(runtime,Runtime::Pump,false,300007);runtime.tick(300008,true);
  assert(!runtime.output[0].command);
  assert(!supervisor.update(runtime,1200006,true,20,true,20,o));
  assert(!runtime.armed&&supervisor.resumeArmed()); // No valve OFF feedback, so do not resume.
  feedback(runtime,0,false,1200007);runtime.tick(1200008,true);
  assert(!supervisor.update(runtime,1200009,true,20,true,20,o));
  assert(runtime.armed&&!supervisor.resumeArmed());

  std::cout<<"Pump thermal guard: five-minute cold retries, warm recovery, return dwell, feedback window, missing readings and timer wrap passed\n";
}
