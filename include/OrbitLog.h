#pragma once
#include <Arduino.h>
#include <memory>
// Application serial output, copied to a bounded RAM ring. Never log credentials.
class OrbitSerial : public Stream {
 static constexpr size_t Capacity=8192;
 char data_[Capacity]={};uint32_t total_=0;
 portMUX_TYPE mux_=portMUX_INITIALIZER_UNLOCKED;
public:
 void begin(unsigned long baud){Serial.begin(baud);}
 int available()override{return Serial.available();}
 int read()override{return Serial.read();}
 int peek()override{return Serial.peek();}
 void flush()override{Serial.flush();}
 using Print::write;
 size_t write(uint8_t c)override{
  const size_t n=Serial.write(c);
  portENTER_CRITICAL(&mux_);data_[total_%Capacity]=char(c);++total_;portEXIT_CRITICAL(&mux_);return n;
 }
 String since(uint32_t& cursor){
  std::unique_ptr<char[]> copy(new(std::nothrow) char[Capacity+1]);if(!copy)return "Log indisponivel: memoria";
  portENTER_CRITICAL(&mux_);uint32_t n=total_-cursor;if(n>Capacity)n=Capacity;
  for(uint32_t i=0;i<n;++i){copy[i]=data_[(total_-n+i)%Capacity];}
  cursor=total_;portEXIT_CRITICAL(&mux_);
  copy[n]=0;return String(copy.get());
 }
 String snapshot(){uint32_t cursor=0;return since(cursor);}
};
inline OrbitSerial& orbitSerial(){static OrbitSerial instance;return instance;}
