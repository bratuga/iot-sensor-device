#include "HeatingRuntime.h"
#include <cassert>
#include <iostream>
using namespace heating;
static void sent(Runtime&r,unsigned i,uint32_t now){auto&a=r.output[i];a.sent=true;a.acknowledged=true;a.changed=now;r.report(i,a.command,now);}
static void boot(Runtime&r,uint32_t now=0){r.openMs=100;r.timeoutMs=50;r.freshnessMs=1000;r.tick(now,true);sent(r,8,now);r.tick(now,true);for(unsigned i=0;i<8;++i)sent(r,i,now);assert(r.arm(now));}
int main(){
 {RuntimeLayout l;assert(l.valid(0));l.zone[0]=0;assert(!l.valid(0));assert(l.valid(1));l.zone[7]=0;assert(l.valid(1));l.position[0]=0;l.position[7]=0;assert(!l.valid(1));l.position[7]=7;assert(l.valid(1));l.zone[7]=-1;assert(l.valid(1));}

 for(unsigned mask=1;mask<256;++mask){Runtime r;boot(r);for(unsigned i=0;i<8;++i)r.demand[i]=(mask>>i)&1;r.tick(1,true);for(unsigned i=0;i<8;++i)sent(r,i,1);r.tick(100,true);assert(!r.output[8].command);r.tick(101,true);assert(r.output[8].command);sent(r,8,101);r.stop(102);r.tick(102,true);for(unsigned i=0;i<8;++i)assert(r.output[i].command==bool((mask>>i)&1));sent(r,8,103);r.tick(103,true);for(unsigned i=0;i<8;++i)assert(!r.output[i].command);}
 {Runtime r;boot(r);r.demand[0]=true;r.tick(1,true);r.output[0].sent=true;r.report(0,true,2);assert(!r.output[0].known);r.tick(51,true);assert(r.fault&&!r.output[8].command);}
 {Runtime r;boot(r);r.demand[0]=true;r.tick(1,true);sent(r,0,1);r.tick(101,true);sent(r,8,101);r.tick(102,false);assert(r.fault&&!r.armed&&!r.output[8].command&&r.output[0].command);r.tick(103,true);assert(!r.arm(103));sent(r,8,104);r.tick(104,true);assert(!r.output[0].command);assert(r.arm(104));assert(!r.demand[0]);}
 {Runtime r;boot(r,0xfffffff0u);r.demand[0]=true;r.tick(0xfffffff1u,true);sent(r,0,0xfffffff1u);r.tick(0x55u,true);assert(r.output[8].command);}
 {Runtime r;boot(r);r.tick(100,true);assert(!r.output[8].command);}
 {Runtime r;boot(r);r.demand[0]=true;r.tick(1,true);sent(r,0,1);r.tick(101,true);sent(r,8,101);r.demand[0]=false;r.demand[7]=true;r.tick(102,true);assert(!r.output[8].command&&r.output[0].command&&!r.output[7].command);sent(r,8,103);r.tick(103,true);assert(!r.output[0].command&&r.output[7].command);sent(r,0,103);sent(r,7,103);r.tick(202,true);assert(!r.output[8].command);r.tick(203,true);assert(r.output[8].command);sent(r,8,203);r.tick(1200,true);assert(r.fault&&!r.output[8].command&&r.output[7].command);}
 std::cout<<"Heating runtime: 255 demand combinations, delays, stop ordering, missing feedback, reconnect and wrap passed\n";
}
