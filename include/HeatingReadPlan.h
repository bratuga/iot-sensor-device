#pragma once
namespace heating {
// One bounded sweep per explicit start. Completing it never schedules another.
struct ReadSweep {
  unsigned index=8, remaining=0, count=9;
  void start(unsigned rooms) { count=9+3*rooms;index=8;remaining=count; }
  void complete() {
    if(!remaining)return;
    index=(index+1)%count;
    --remaining;
  }
};
}
