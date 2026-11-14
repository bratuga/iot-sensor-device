#pragma once
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
uint32_t millis();
void configTzTime(const char*,const char*,const char*);

#ifdef _WIN32
inline tm* localtime_r(const time_t* t,tm* out){return localtime_s(out,t)==0?out:nullptr;}
#endif
