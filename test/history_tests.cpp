#include "History.h"
#include <cassert>
#include <iostream>
#include <limits>
int main(){using namespace history;Data d{};Record r{};r.time=MinTime;r.value[0]=encode(21.125f,true);r.value[1]=encode(0,false);r.value[2]=encode(std::numeric_limits<float>::quiet_NaN(),true);r.value[3]=encode(55.2f,true);r.value[4]=1;
assert(r.value[0]==2113 && r.value[1]==Missing && r.value[2]==Missing && r.value[3]==5520 && r.value[4]==1);
assert(add(d,r));assert(!add(d,r));for(unsigned i=1;i<145;++i){r.time=MinTime+i*Interval;assert(add(d,r));}assert(d.count==144);assert(d.records[0].time==MinTime+Interval);assert(visible(r.time,r.time));assert(!visible(r.time,r.time+Retention));assert(!visible(r.time,r.time-1));
std::cout<<"PASS: history capacity, sample interval, missing readings and 48-hour retention\n";}
