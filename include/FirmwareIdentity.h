#pragma once
#include "OtaGuard.h"
#define IOT_FIRMWARE_PRODUCT "iot-sensor-device"
#define IOT_FIRMWARE_HARDWARE "sensor-esp32"
#define IOT_FIRMWARE_VERSION "0.3.22-orbit-ntp-ota"
#define IOT_FIRMWARE_CHIP 0
extern "C" { extern const iotfw::Identity iot_firmware_identity; }
