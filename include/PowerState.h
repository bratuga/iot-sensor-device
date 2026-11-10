#pragma once
#include <stdint.h>
namespace device {
struct PowerState {
 uint32_t magic=0x4f505731;
 uint8_t paused=0, relays=0, reserved[2]={};
 uint32_t identity[4]={};
 bool valid() const {return magic==0x4f505731&&paused<=1&&relays<=15&&!reserved[0]&&!reserved[1];}
 bool restoreRelay(unsigned i,uint32_t currentIdentity) const {
  return i<4&&(relays&(1u<<i))&&identity[i]==currentIdentity;
 }
};
inline uint32_t powerIdentity(const char* text) {
 uint32_t value=2166136261u;
 while(*text){value^=uint8_t(*text++);value*=16777619u;}
 return value;
}
}
