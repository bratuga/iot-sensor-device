#pragma once
#include <cstdint>
#include <cstddef>
#include <cmath>
namespace history {
constexpr unsigned Capacity=144;
constexpr uint32_t Interval=1200, Retention=172800, MinTime=1577836800;
constexpr int16_t Missing=-32768;
struct Record { uint32_t time; int16_t value[5]; };
struct Data { uint32_t count=0; Record records[Capacity]={}; };
inline int16_t encode(float x, bool valid) {
  return valid && std::isfinite(x) && x>=-100 && x<=200 ? int16_t(std::lround(x*100)) : Missing;
}
inline bool add(Data &d,const Record &r) {
  if(r.time<MinTime || (d.count && r.time/Interval<=d.records[d.count-1].time/Interval)) return false;
  if(d.count>=Capacity){for(unsigned i=1;i<Capacity;i++)d.records[i-1]=d.records[i];d.count=Capacity-1;}
  d.records[d.count++]=r;return true;
}
inline bool visible(uint32_t timestamp,uint32_t now){return timestamp<=now && now-timestamp<Retention;}
}
