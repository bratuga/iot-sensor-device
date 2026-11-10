#pragma once
#include <Arduino.h>
#include <esp_heap_caps.h>
// Arduino ESP32 2.0.17 ships without FreeRTOS runtime counters. Report the
// measured heap percentage and actual CPU frequency, never a fabricated load.
template<class Json> void orbitResources(Json& d){
 const uint32_t caps=MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT;
 const size_t total=heap_caps_get_total_size(caps),free=heap_caps_get_free_size(caps);
 d["cpuUsePct"]=nullptr;d["cpuMHz"]=getCpuFrequencyMhz();
 if(total&&free<=total)d["heapUsePct"]=unsigned((100*(total-free)+total/2)/total);
 else d["heapUsePct"]=nullptr;
}
