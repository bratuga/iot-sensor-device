#include "PowerState.h"
#include "HeatingRuntime.h"
#include <cassert>
#include <cstring>
using namespace heating;
void confirm(Runtime& r,unsigned i,bool on,uint32_t now){
 r.output[i].sent=true;r.acknowledge(i,true);r.reportWhileOff(i,on,now);
}
int main(){
 device::PowerState saved;saved.paused=1;saved.relays=5;
 saved.identity[0]=device::powerIdentity("13:1:1/1/1:1/1/2");
 unsigned char nvs[sizeof(saved)];memcpy(nvs,&saved,sizeof(saved));
 device::PowerState reboot;memcpy(&reboot,nvs,sizeof(reboot));
 assert(reboot.valid()&&reboot.paused&&reboot.relays==5);
 assert(reboot.restoreRelay(0,device::powerIdentity("13:1:1/1/1:1/1/2")));
 assert(!reboot.restoreRelay(0,device::powerIdentity("14:1:1/1/1:1/1/2")));
 assert(!reboot.restoreRelay(0,device::powerIdentity("13:1:2/1/1:1/1/2")));
 reboot.magic=0;assert(!reboot.valid());
 Runtime r;RuntimeLayout layout;layout.zone[0]=0;r.configure(layout);r.tick(0,true);
 confirm(r,8,false,1);r.tick(1,true);confirm(r,0,false,2);assert(r.arm(3));
 r.demand[0]=true;r.tick(4,true);r.output[0].sent=true;r.acknowledge(0,true);r.report(0,true,5);
 r.openMs=1;r.tick(10,true);r.output[8].sent=true;r.acknowledge(8,true);r.report(8,true,11);
 r.stop(12);assert(!r.armed);assert(r.nextCommand(12)==8);
 // Valve remains open until fresh pump OFF ACK plus feedback.
 r.output[8].sent=true;r.reportWhileOff(8,false,13);r.tick(13,true);assert(r.output[0].command);
 r.acknowledge(8,true);r.tick(14,true);assert(!r.output[0].command&&r.nextCommand(14)==0);
 confirm(r,0,false,15);assert(r.canArm(15));
 // An external ON during master OFF cannot be accepted as an external path.
 r.reportWhileOff(0,true,16);r.tick(16,true);
 assert(!r.external[0]&&!r.confirmed(0,false,16)&&r.nextCommand(16)==0);
 confirm(r,0,false,17);assert(r.canArm(17));
 r.reportWhileOff(8,true,18);r.tick(18,true);assert(r.nextCommand(18)==8&&!r.canArm(18));
}
