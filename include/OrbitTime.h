#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <esp_sntp.h>
#include <atomic>
#include <time.h>

namespace orbitTime {
extern std::atomic<uint32_t> sequence;
extern std::atomic<uint32_t> lastSync;
extern uint32_t requestAt,requestSequence;
extern bool requested;
inline void notified(timeval* value){lastSync.store(value->tv_sec);sequence.fetch_add(1);}
inline void begin(const char* timezone,const char* primary,const char* secondary){
 sntp_set_time_sync_notification_cb(notified);configTzTime(timezone,primary,secondary);
}
inline bool valid(){return time(nullptr)>=1704067200;}
inline bool pending(){return requested&&sequence.load()==requestSequence&&uint32_t(millis()-requestAt)<30000;}
inline const char* state(){
 if(pending())return "pending";
 if(requested&&sequence.load()==requestSequence)return "timeout";
 return lastSync.load()?"synced":"idle";
}
inline bool synchronize(const char* timezone,const char* primary,const char* secondary){
 if(!WiFi.isConnected()||pending())return false;
 requestSequence=sequence.load();requestAt=millis();requested=true;
 begin(timezone,primary,secondary);return true;
}
template<class Json> void status(Json& d){
 time_t now=time(nullptr);tm local{};localtime_r(&now,&local);char label[40];strftime(label,sizeof(label),"%Y-%m-%d %H:%M:%S %Z",&local);d["localTime"]=valid()?label:"";
 d["unix"]=int64_t(now);d["valid"]=valid();d["syncState"]=state();
 d["lastSync"]=int64_t(lastSync.load());d["syncSequence"]=sequence.load();
 d["rtcAvailable"]=false;d["storageAvailable"]=false;
}
}
