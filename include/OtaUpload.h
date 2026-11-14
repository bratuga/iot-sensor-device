#pragma once
#include <Update.h>
#include "FirmwareIdentity.h"
struct OrbitUpdateWriter {
 bool begin(size_t size){return Update.begin(size,U_FLASH);}
 size_t write(const uint8_t* data,size_t size){return Update.write(const_cast<uint8_t*>(data),size);}
 bool end(){return Update.end(false);}
 void abort(){Update.abort();}
};
static OrbitUpdateWriter orbitUpdateWriter;
static iotfw::Upload<OrbitUpdateWriter> otaUpload(orbitUpdateWriter,IOT_FIRMWARE_PRODUCT,IOT_FIRMWARE_HARDWARE,IOT_FIRMWARE_CHIP);
