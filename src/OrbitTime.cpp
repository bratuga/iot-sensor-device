#include "OrbitTime.h"
namespace orbitTime {
std::atomic<uint32_t> sequence{0},lastSync{0};
uint32_t requestAt=0,requestSequence=0;
bool requested=false;
}
