#include "FirmwareIdentity.h"
#include <cassert>
#include <vector>
#include <iostream>
struct Writer {
 int begins=0,ends=0,aborts=0;size_t bytes=0;bool allow=true,endOk=true,writeOk=true;
 bool begin(size_t){++begins;return allow;}
 size_t write(const uint8_t*,size_t n){bytes+=n;return writeOk?n:0;}
 bool end(){++ends;return endOk;}
 void abort(){++aborts;}
};
std::vector<uint8_t> image(){
 std::vector<uint8_t> v(1024,0);v[0]=0xe9;v[1]=2;v[12]=IOT_FIRMWARE_CHIP;v[28]=0x00;v[29]=0x02;
 v[32]=0x32;v[33]=0x54;v[34]=0xcd;v[35]=0xab;
 iotfw::Identity identity={"IOTFW01",IOT_FIRMWARE_PRODUCT,IOT_FIRMWARE_HARDWARE,IOT_FIRMWARE_VERSION};
 memcpy(v.data()+288,&identity,sizeof(identity));return v;
}
using Upload=iotfw::Upload<Writer>;
int main(){
 const auto good=image();
 for(size_t split=1;split<iotfw::PrefixSize;++split){
  Writer w;Upload u(w,IOT_FIRMWARE_PRODUCT,IOT_FIRMWARE_HARDWARE,IOT_FIRMWARE_CHIP);
  assert(u.start(good.size()));assert(u.write(good.data(),split));assert(w.begins==0&&w.bytes==0);
  assert(u.write(good.data()+split,good.size()-split));assert(w.begins==1&&w.ends==0);
  assert(u.finish());assert(w.ends==1&&w.bytes==good.size());assert(!u.finish());
 }
 for(int offset:{0,12,32,288,296,328,360}){
  auto bad=good;bad[offset]=0; if(offset==12)bad[offset]=IOT_FIRMWARE_CHIP==0?9:0;
  Writer w;Upload u(w,IOT_FIRMWARE_PRODUCT,IOT_FIRMWARE_HARDWARE,IOT_FIRMWARE_CHIP);
  assert(u.start(bad.size()));assert(!u.write(bad.data(),bad.size()));assert(!u.finish());assert(w.begins==0&&w.bytes==0&&w.ends==0);
 }
 for(int mode=0;mode<5;++mode){
  Writer w;Upload u(w,IOT_FIRMWARE_PRODUCT,IOT_FIRMWARE_HARDWARE,IOT_FIRMWARE_CHIP);
  if(mode==0){assert(!u.start(0));assert(!u.finish());}
  if(mode==1){assert(u.start(good.size()+1));assert(u.write(good.data(),good.size()));assert(!u.finish());}
  if(mode==2){assert(u.start(good.size()-1));assert(!u.write(good.data(),good.size()));assert(!u.finish());}
  if(mode==3){assert(u.start(good.size()));assert(u.write(good.data(),good.size()));u.abort();assert(!u.finish());}
  if(mode==4){w.writeOk=false;assert(u.start(good.size()));assert(!u.write(good.data(),good.size()));assert(!u.finish());}
  assert(w.ends==0);
 }
 Writer w;Upload u(w,IOT_FIRMWARE_PRODUCT,IOT_FIRMWARE_HARDWARE,IOT_FIRMWARE_CHIP);
 w.endOk=false;assert(u.start(good.size()));assert(u.write(good.data(),good.size()));assert(!u.finish());assert(w.aborts==1);
 std::cout<<"PASS: split prefixes, wrong device/chip/legacy headers, truncation, overflow, abort, write failure, rejected final image\n";
}
