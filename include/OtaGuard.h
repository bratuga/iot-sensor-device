#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// ESP-IDF: image header (24), first segment header (8), app descriptor (256).
// This custom descriptor is linked immediately after esp_app_desc_t.
namespace iotfw {
struct Identity { char magic[8]; char product[32]; char hardware[32]; char version[32]; };
static_assert(sizeof(Identity)==104,"Stable OTA descriptor");
constexpr size_t IdentityOffset=288, PrefixSize=IdentityOffset+sizeof(Identity);
inline uint32_t little32(const uint8_t* p){return uint32_t(p[0])|uint32_t(p[1])<<8|uint32_t(p[2])<<16|uint32_t(p[3])<<24;}
inline bool textField(const char* p,size_t n){
 const char* end=static_cast<const char*>(memchr(p,0,n));
 if(!end||end==p)return false;
 for(const char* c=p;c<end;++c)if(*c<32||*c>126)return false;
 return true;
}
inline const char* validate(const uint8_t* p,const char* product,const char* hardware,uint16_t chip){
 if(p[0]!=0xe9||p[1]<1||p[1]>16||little32(p+32)!=0xabcd5432||little32(p+28)<PrefixSize-32)
  return "Ficheiro de aplicacao ESP invalido";
 if((uint16_t(p[12])|(uint16_t(p[13])<<8))!=chip)return "Processador incompativel";
 Identity id{};memcpy(&id,p+IdentityOffset,sizeof(id));
 if(memcmp(id.magic,"IOTFW01",8)||!textField(id.product,32)||!textField(id.hardware,32)||!textField(id.version,32))
  return "Firmware sem identificacao compativel; usar uma compilacao recente";
 if(strcmp(id.product,product)||strcmp(id.hardware,hardware))return "Firmware de outro equipamento ou perfil de hardware";
 return nullptr;
}
// Writer begins only AFTER the complete identity has been validated. finish()
// must be called by the authenticated HTTP completion handler, never a chunk callback.
template<class Writer> class Upload {
 Writer& writer_; const char* product_;const char* hardware_;uint16_t chip_;
 uint8_t prefix_[PrefixSize]{};size_t used_=0,expected_=0,received_=0;
 bool started_=false,active_=false;const char* error_="Atualizacao nao iniciada";
 bool fail(const char* message){if(started_)writer_.abort();started_=active_=false;error_=message;return false;}
public:
 Upload(Writer& writer,const char* product,const char* hardware,uint16_t chip):writer_(writer),product_(product),hardware_(hardware),chip_(chip){}
 bool start(size_t expected){
  if(started_){writer_.abort();}
  started_=false;used_=received_=0;expected_=expected;error_=nullptr;active_=true;
  if(expected<PrefixSize||expected>16*1024*1024)return fail("Tamanho de firmware invalido");
  return true;
 }
 bool write(const uint8_t* data,size_t size){
  if(!active_||error_)return false;
  if(size>expected_-received_)return fail("Ficheiro maior que o tamanho indicado");
  received_+=size;
  if(used_<PrefixSize){
   const size_t take=size<PrefixSize-used_?size:PrefixSize-used_;memcpy(prefix_+used_,data,take);used_+=take;data+=take;size-=take;
   if(used_<PrefixSize)return true;
   if(const char* reason=validate(prefix_,product_,hardware_,chip_))return fail(reason);
   if(!writer_.begin(expected_))return fail("Sem espaco ou falha ao iniciar OTA");
   started_=true;
   if(writer_.write(prefix_,PrefixSize)!=PrefixSize)return fail("Falha de escrita OTA");
  }
  if(size&&writer_.write(data,size)!=size)return fail("Falha de escrita OTA");
  return true;
 }
 bool finish(){
  if(error_||!active_)return false;
  if(!started_||received_!=expected_)return fail("Firmware incompleto");
  if(!writer_.end())return fail("Firmware corrompido ou imagem invalida");
  active_=started_=false;return true;
 }
 void abort(){fail("Atualizacao interrompida");}
 const char* error()const{return error_?error_:"";}
};
}
