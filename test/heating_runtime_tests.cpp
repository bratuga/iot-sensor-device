#include "HeatingRuntime.h"
#include "HeatingSnapshot.h"
#include "HeatingReadPlan.h"
#include <cassert>
#include <iostream>
using namespace heating;
static void sent(Runtime&r,unsigned i,uint32_t now){auto&a=r.output[i];a.sent=true;a.acknowledged=true;a.changed=now;r.report(i,a.command,now);}
static void boot(Runtime&r,uint32_t now=0){r.openMs=100;r.timeoutMs=50;r.tick(now,true);sent(r,8,now);r.tick(now,true);for(unsigned i=0;i<8;++i)sent(r,i,now);assert(r.arm(now));}
int main(){
 {ZoneInput z;z.source=TemperatureSource::Knx;z.maxAgeMs=10000;z.hysteresis=1;z.temperature=19;z.setpoint=20;z.valid=z.setpointValid=true;
  assert(roomDemand(true,0,true,false,z,0,false));
  z.temperature=20;assert(roomDemand(true,0,true,false,z,0,true));assert(!roomDemand(true,0,true,false,z,0,false));
  z.temperature=20.5f;assert(!roomDemand(true,0,true,false,z,0,true));
  z.temperature=19;assert(!roomDemand(true,0,true,false,z,10000,true));
  assert(!roomDemand(true,0,false,true,z,10000,false));assert(roomDemand(true,0,true,true,z,10000,false));
  assert(roomDemand(true,2,false,false,z,10000,false));assert(!roomDemand(false,2,true,true,z,0,true));
  assert(!roomDemand(true,1,true,true,z,0,true));
  assert(!manualExpired(2,1799999,0));assert(manualExpired(2,1800000,0));assert(!manualExpired(0,1800000,0));
  assert(manualExpired(2,uint32_t(0xfffffff0u+1800000u),0xfffffff0u));
  z.setpoint=23;defaultSetpoint(z,false,300000,0);assert(z.setpoint==23);
  z.setpointValid=false;defaultSetpoint(z,true,300000,0);assert(!z.setpointValid);
  defaultSetpoint(z,false,299999,0);assert(!z.setpointValid);defaultSetpoint(z,false,300000,0);assert(z.setpointValid&&z.setpoint==20);
 }

 {ZoneInput z;z.source=TemperatureSource::Knx;z.maxAgeMs=300000;z.setpoint=20;z.setpointValid=true;z.hysteresis=0.6;
  assert(!selectTemperature(z,true,18,true,299999,299999,0).valid);
  auto f=selectTemperature(z,true,18,true,300000,300000,0);assert(f.source==TemperatureSource::Bme280&&f.demand(300000,false));assert(!z.valid);
  assert(!selectTemperature(z,false,18,true,300000,300000,0).valid);
  assert(!selectTemperature(z,true,18,false,300000,300000,0).valid);
  z.valid=true;z.receivedAt=300001;z.temperature=21;
  f=selectTemperature(z,true,18,true,300001,300001,0);assert(f.source==TemperatureSource::Knx&&!f.demand(300001,true));
  f=selectTemperature(z,true,18,true,600001,600001,0);assert(f.source==TemperatureSource::Bme280);
  z.valid=false;assert(selectTemperature(z,true,18,true,0x493d0,0x493d0,0xfffffff0).valid);
 }

 {RuntimeLayout l;assert(l.valid(0));l.zone[0]=0;assert(!l.valid(0));assert(l.valid(1));l.zone[7]=0;assert(l.valid(1));l.position[0]=0;l.position[7]=0;assert(!l.valid(1));l.position[7]=7;assert(l.valid(1));l.zone[7]=-1;assert(l.valid(1));}

 for(unsigned mask=1;mask<256;++mask){Runtime r;boot(r);for(unsigned i=0;i<8;++i)r.demand[i]=(mask>>i)&1;r.tick(1,true);for(unsigned i=0;i<8;++i)sent(r,i,1);r.tick(100,true);assert(!r.output[8].command);r.tick(101,true);assert(r.output[8].command);sent(r,8,101);r.stop(102);r.tick(102,true);for(unsigned i=0;i<8;++i)assert(r.output[i].command==bool((mask>>i)&1));sent(r,8,103);r.tick(103,true);for(unsigned i=0;i<8;++i)assert(!r.output[i].command);}
 {Runtime r;boot(r);r.demand[0]=true;r.tick(1,true);r.output[0].sent=true;r.report(0,true,2);assert(r.output[0].known&&!r.confirmed(0,true,2));r.tick(51,true);assert(r.fault&&!r.output[8].command);}
 {Runtime r;boot(r);r.demand[0]=true;r.tick(1,true);sent(r,0,1);r.tick(101,true);sent(r,8,101);r.tick(102,false);assert(r.fault&&!r.armed&&!r.output[8].command&&r.output[0].command);r.tick(103,true);assert(!r.arm(103));sent(r,8,104);r.tick(104,true);assert(!r.output[0].command);for(unsigned i=0;i<8;++i)sent(r,i,104);assert(r.arm(104));assert(!r.demand[0]);}
 {Runtime r;boot(r,0xfffffff0u);r.demand[0]=true;r.tick(0xfffffff1u,true);sent(r,0,0xfffffff1u);r.tick(0x55u,true);assert(r.output[8].command);}
 {Runtime r;boot(r);r.tick(100,true);assert(!r.output[8].command);}
 {Runtime r;boot(r);r.demand[0]=true;r.tick(1,true);sent(r,0,1);r.tick(101,true);sent(r,8,101);
  // Add a second room under a running pump; no new pump command is needed.
  r.demand[7]=true;r.tick(102,true);assert(r.output[8].command&&r.output[0].command&&r.output[7].command);
  sent(r,7,103);r.tick(104,true);assert(r.output[8].command&&!r.fault);
  // Closing a path follows the existing pump-OFF feedback sequence.
  r.demand[0]=false;r.tick(105,true);assert(!r.output[8].command&&r.output[0].command);
  sent(r,8,106);r.tick(106,true);assert(!r.output[0].command&&r.output[7].command);
  sent(r,0,107);r.tick(203,true);assert(r.output[8].command&&!r.fault);
  r.tick(204,false);assert(r.fault&&!r.output[8].command&&r.output[7].command);}
 {Runtime r;boot(r);r.demand[0]=true;r.tick(1,true);r.output[0].sent=true;r.report(0,true,2);assert(!r.confirmed(0,true,2));r.acknowledge(0,true);assert(r.confirmed(0,true,3));r.set(0,false,4);assert(!r.output[0].known);r.output[0].sent=true;r.acknowledge(0,true);assert(!r.confirmed(0,false,5));r.report(0,false,6);assert(r.confirmed(0,false,6));r.acknowledge(0,false);assert(!r.confirmed(0,false,7));}
 {Runtime r;r.tick(0,true);r.output[8].sent=true;r.report(8,false,1);assert(!r.arm(1));r.acknowledge(8,true);for(unsigned i=0;i<8;++i)sent(r,i,2);assert(r.arm(2));}
 {Snapshot s;s.hasTemperature=s.hasSetpoint=true;s.temperature=19;s.setpoint=22;ZoneInput z;z.source=TemperatureSource::Knx;z.hysteresis=0.5f;z.maxAgeMs=300000;s.restore(z);assert(z.temperature==19&&z.setpoint==22&&z.setpointValid);assert(!z.valid&&!z.demand(1,false));z.valid=true;z.receivedAt=2;assert(z.demand(2,false));s.setpoint=100;s.restore(z);assert(!z.setpointValid&&!z.valid);}
 {ReadSweep sweep;assert(!sweep.remaining);for(unsigned rooms=0;rooms<=16;++rooms){sweep.start(rooms);assert(sweep.index==8);unsigned reads=0;bool seen[57]={};for(unsigned tick=0;tick<10000;++tick){if(sweep.remaining){assert(!seen[sweep.index]);seen[sweep.index]=true;++reads;sweep.complete();}}assert(reads==9+3*rooms&&!sweep.remaining);sweep.complete();assert(!sweep.remaining);}}
 std::cout<<"Heating runtime: 255 demand combinations, delays, stop ordering, missing feedback, reconnect and wrap passed\n";
}
