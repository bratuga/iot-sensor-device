#include "HeatingRuntime.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>
using namespace heating;
struct Event { uint32_t at; unsigned i; bool value; bool ack; unsigned generation; };
struct Sim {
 Runtime r;
 uint32_t now=0, lastSend=uint32_t(-200), delay=100;
 bool physical[9]={};uint32_t energized[9]={};unsigned generation[9]={}, writes[9]={};
 int drop=-1, dropAck=-1; bool ackFirst=true, accept=true, autoFeedback=true;
 std::vector<Event> events;
 Sim(unsigned mask=255) {
  RuntimeLayout l;for(unsigned i=0;i<8;++i)if(mask&(1u<<i))l.zone[i]=i%5;
  r.configure(l);r.openMs=120000;r.timeoutMs=30000;
 }
 void deliver(unsigned i,bool value){r.report(i,value,now);}
 void step(uint32_t t,bool link=true) {
  now=t;
  for(auto it=events.begin();it!=events.end();) {
   if(int32_t(now-it->at)<0){++it;continue;}
   auto e=*it;it=events.erase(it);
   if(e.ack){if(e.generation==generation[e.i])r.acknowledge(e.i,true);}
   else deliver(e.i,e.value);
  }
  r.tick(now,link);
  if(!link)return;
  if(age(now,lastSend)>=200 && accept){int n=r.nextCommand(now);if(n>=0){
   unsigned i=unsigned(n);assert(r.active(i));auto &a=r.output[i];
   if(i==8&&a.command){bool path=false;for(unsigned j=0;j<8;++j)path|=physical[j]&&age(now,energized[j])>=r.openMs;assert(path);}
   if(i<8&&!a.command&&physical[i])assert(!physical[8]);
   a.sent=true;a.acknowledged=false;a.changed=now;lastSend=now;++writes[i];++generation[i];
   if(a.command&&!physical[i])energized[i]=now;
   physical[i]=a.command;
   if(int(i)!=dropAck)events.push_back({now+(ackFirst?10:delay+10),i,a.command,true,generation[i]});
   if(autoFeedback&&int(i)!=drop){
    uint32_t feedbackAt=now+delay;
    // A working bus preserves telegram order from the same actuator output.
    for(const auto &e:events)if(!e.ack&&e.i==i&&int32_t(e.at-feedbackAt)>=0)feedbackAt=e.at+10;
    events.push_back({feedbackAt,i,a.command,false,generation[i]});
   }
  }}
 }
 void run(uint32_t until){for(uint32_t t=now+10;t<=until;t+=10)step(t);}
 void boot(){step(0);run(3000);assert(r.canArm(now));assert(r.arm(now));}
};
int main(int argc,char **argv){
 // Every assignment mask and every demand subset: 3^8 combinations, including no rooms.
 unsigned cases=0;
 for(unsigned used=0;used<256;++used)for(unsigned wanted=used;;wanted=(wanted-1)&used){
  Sim s(used);s.r.openMs=1000;s.boot();
  for(unsigned i=0;i<8;++i)s.r.demand[i]=(wanted>>i)&1;
  s.run(7000);assert(!s.r.fault);assert(s.r.confirmed(8,wanted!=0,s.now));
  for(unsigned i=0;i<8;++i)if(!(used&(1u<<i)))assert(s.writes[i]==0);
  s.r.stop(s.now);s.run(10000);assert(!s.physical[8]);
  for(unsigned i=0;i<8;++i)assert(!s.physical[i]);
  ++cases;if(!wanted)break;
 }
 assert(cases==6561);
 // Real capture prefix: spare C1 never replies; WC C4 turns ON around 28 s.
 assert(argc==2);std::ifstream f(argv[1]);assert(f);std::vector<Event> recorded;
 unsigned t,i,v;while(f>>t>>i>>v)recorded.push_back({t,i,v!=0,false,0});assert(recorded.size()>20);
 Sim replay(254);replay.autoFeedback=false;replay.step(0);unsigned cursor=0;
 for(t=10;t<=30210;t+=10){
  replay.now=t;while(cursor<recorded.size()&&recorded[cursor].at<=t){auto e=recorded[cursor++];replay.deliver(e.i,e.value);}
  replay.step(t);
  if(t==22000)assert(replay.r.arm(t));
  if(t==28040)replay.r.demand[3]=true;
 }
 assert(!replay.r.fault&&replay.r.output[3].command&&replay.writes[0]==0);
 // Extend the capture with a responding actuator rather than the OLD shutdown's consequences.
 replay.autoFeedback=true;replay.run(155000);assert(!replay.r.fault&&replay.r.confirmed(8,true,replay.now));
 replay.r.stop(replay.now);replay.run(158000);assert(!replay.physical[8]&&!replay.physical[3]);
 // ACK alone never confirms; delayed, duplicate or reversed ACK/feedback order works.
 for(bool first:{false,true}){Sim s(8);s.ackFirst=first;s.delay=27000;s.step(0);s.run(58000);assert(s.r.arm(s.now));s.r.demand[3]=true;s.run(85000);assert(!s.r.output[8].command);s.run(90000);for(int k=0;k<4;++k)s.r.report(3,true,s.now);s.run(240000);assert(!s.r.fault&&s.r.confirmed(8,true,s.now));}
 {Sim s(8);s.drop=3;s.step(0);s.run(35000);assert(!s.r.arm(s.now));assert(s.r.faultOutput==3&&s.r.failure==Runtime::Failure::FeedbackTimeout);}
 {Sim s(8);s.drop=8;s.step(0);s.run(35000);assert(s.r.faultOutput==8);assert(s.writes[3]==0);}
 {Sim s(8);s.boot();s.drop=3;s.r.demand[3]=true;s.run(34000);assert(s.r.faultOutput==3&&!s.physical[8]);}
 {Sim s(8);s.boot();s.dropAck=3;s.r.demand[3]=true;s.run(34000);assert(s.r.faultOutput==3&&!s.physical[8]);}
 // Pump timeout stops pump before releasing an open path; no fabricated confirmation.
 {Sim s(8);s.r.openMs=1000;s.boot();s.drop=8;s.r.demand[3]=true;s.run(37000);assert(s.r.faultOutput==8&&s.physical[3]);assert(!s.r.confirmed(8,false,s.now));s.drop=-1;s.deliver(8,false);s.run(40000);assert(!s.physical[3]);}
 // Wrong feedback is not confirmation; transport failure carries the actual output.
 {Sim s(8);s.boot();s.drop=3;s.r.demand[3]=true;s.run(4000);s.deliver(3,false);s.run(34000);assert(s.r.faultOutput==3&&!s.physical[8]);}
 {Sim s(8);s.boot();s.r.demand[3]=true;s.run(4000);s.r.acknowledge(3,false);s.r.trip(s.now,Runtime::Failure::Transport,3);s.run(7000);assert(s.r.failure==Runtime::Failure::Transport&&s.r.faultOutput==3&&!s.physical[8]);}
 // Queue full: no sent marker or false confirmation; recover without discarding demand.
 {Sim s(8);s.boot();s.accept=false;s.r.demand[3]=true;s.run(40000);assert(!s.r.output[3].sent&&!s.physical[8]);s.accept=true;s.run(165000);assert(s.r.confirmed(8,true,s.now));}
 // Network loss/reconnection does not restart automatically; startup OFF remains ordered.
 {Sim s(8);s.r.openMs=1000;s.boot();s.r.demand[3]=true;s.run(6000);s.step(6010,false);assert(s.r.fault&&!s.r.armed&&s.physical[3]);s.step(6020,true);s.run(10000);assert(!s.physical[8]&&!s.physical[3]);assert(s.r.arm(s.now));}
 // Unsolicited pump ON is answered with OFF and a specific fault, while paths stay open.
 {Sim s(8);s.boot();s.physical[8]=true;s.deliver(8,true);s.step(3010);assert(s.r.failure==Runtime::Failure::UnexpectedPump&&!s.r.armed);s.run(4000);assert(s.r.confirmed(8,false,s.now));}
 // A second room joins: keep pump continuously ON and command only its valve.
 {Sim s(136);s.r.openMs=1000;s.boot();s.r.demand[3]=true;s.run(6000);
  assert(s.physical[8]);unsigned pumpWrites=s.writes[8];s.r.demand[7]=true;
  for(uint32_t t=6010;t<=9000;t+=10){s.step(t);assert(s.physical[8]&&!s.r.fault);}
  assert(s.physical[3]&&s.physical[7]&&s.writes[8]==pumpWrites);
  // Closing one path still waits for pump OFF feedback.
  s.r.demand[3]=false;s.run(11000);
  assert(!s.r.fault&&s.physical[7]&&!s.physical[3]&&s.physical[8]&&s.writes[8]==pumpWrites+2);
 }
 {Sim s(136);s.r.openMs=1000;s.boot();s.r.demand[3]=true;s.run(6000);
  unsigned pumpWrites=s.writes[8];s.r.demand[3]=false;s.r.demand[7]=true;
  for(uint32_t t=6010;t<=7200;t+=10){s.step(t);assert(!s.r.fault);}
  s.run(8500);assert(!s.physical[3]&&s.physical[7]&&s.physical[8]&&s.writes[8]==pumpWrites+2);
 }
 // Seeded changing demand with asynchronous feedback: no pump/path invariant violations.
 for(unsigned seed=1;seed<=32;++seed){
  Sim s(254);s.r.openMs=1000;s.boot();unsigned random=seed;
  for(uint32_t t=3010;t<=120000;t+=10){
   if(t%1700==0){random=random*1664525u+1013904223u;
    for(unsigned i=1;i<8;++i)s.r.demand[i]=(random>>i)&1;
    s.delay=50+(random%300);s.ackFirst=(random&1)!=0;
   }
   s.step(t);

   if(s.r.fault)std::cerr<<"seed="<<seed<<" t="<<t<<" failure="<<int(s.r.failure)<<" output="<<s.r.faultOutput<<"\n";
   assert(!s.r.fault);
  }
  s.r.stop(s.now);s.run(125000);assert(!s.physical[8]);
 }
 std::cout<<"PASS simulation: 6561 usage/demand combinations, v4 capture replay + extended cycle, late/absent feedback and ACK, queue full, network loss, external pump ON, room transitions, 32 seeded event sequences\n";
}
