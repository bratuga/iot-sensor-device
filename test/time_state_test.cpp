#include "OrbitTime.h"
#include <cassert>
#include <cstring>
#include <iostream>
uint32_t fakeMillis=0;bool fakeConnected=true;int requests=0;
void (*callback)(timeval*)=nullptr;
WifiFake WiFi;
uint32_t millis(){return fakeMillis;}
void configTzTime(const char*,const char*,const char*){++requests;}
void sntp_set_time_sync_notification_cb(void(*cb)(timeval*)){callback=cb;}
int main(){
 assert(orbitTime::valid());assert(!strcmp(orbitTime::state(),"idle"));
 fakeConnected=false;assert(!orbitTime::synchronize("UTC0","one","two"));assert(requests==0);
 fakeConnected=true;assert(orbitTime::synchronize("UTC0","one","two"));assert(!strcmp(orbitTime::state(),"pending"));
 assert(!orbitTime::synchronize("UTC0","one","two"));assert(requests==1);
 fakeMillis=30000;assert(!strcmp(orbitTime::state(),"timeout"));
 assert(orbitTime::synchronize("UTC0","one","two"));timeval tv{1800000000,0};callback(&tv);
 assert(!strcmp(orbitTime::state(),"synced"));assert(orbitTime::lastSync.load()==1800000000);
 fakeMillis=0xfffffff0;assert(orbitTime::synchronize("UTC0","one","two"));fakeMillis=1000;assert(orbitTime::pending());fakeMillis=40000;assert(!strcmp(orbitTime::state(),"timeout"));
 std::cout<<"PASS: valid clock is not NTP success; Wi-Fi rejection, pending, timeout, actual callback and millis wrap\n";
}
