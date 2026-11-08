#include "ReadingTrend.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
  sensor::ReadingTrend trend;
  constexpr uint32_t start=0xffff0000u, minute=60000;
  float delta=0;
  for(unsigned i=0;i<30;++i)trend.sample(start+i*minute,20+i*0.1f,true);
  assert(!trend.delta(start+29*minute,22.9f,delta));
  trend.sample(start+30*minute,23,true);
  assert(trend.delta(start+30*minute,23,delta)&&std::fabs(delta-3)<0.001f);
  for(unsigned i=31;i<70;++i)trend.sample(start+i*minute,20+i*0.1f,true);
  assert(trend.delta(start+69*minute,26.9f,delta)&&std::fabs(delta-3)<0.001f);
  assert(!trend.delta(start+120*minute,30,delta));
  trend.sample(start+70*minute,0,false);
  assert(!trend.delta(start+70*minute,20,delta));
  std::cout<<"Trend: rolling 30-minute comparison, wrap, stale and missing readings passed\n";
}
