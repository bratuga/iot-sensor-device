#include "FirmwareIdentity.h"
extern "C" {
extern const iotfw::Identity iot_firmware_identity __attribute__((section(".rodata_custom_desc"),used)) = {
 "IOTFW01", IOT_FIRMWARE_PRODUCT, IOT_FIRMWARE_HARDWARE, IOT_FIRMWARE_VERSION
};
}
