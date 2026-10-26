#pragma once
#include <cstddef>
#include <cstdint>

namespace configjournal {
constexpr uint32_t Magic = 0x31474643; // "CFG1"
constexpr size_t MaxLength = 12000;
struct Header {
  uint32_t magic, sequence, length, checksum;
};
inline uint32_t crc32(const char *bytes, size_t length) {
  uint32_t crc = ~uint32_t(0);
  for (size_t i = 0; i < length; ++i) {
    crc ^= uint8_t(bytes[i]);
    for (unsigned j = 0; j < 8; ++j)
      crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320UL : 0);
  }
  return ~crc;
}
inline unsigned newest(bool valid0, uint32_t seq0, bool valid1, uint32_t seq1) {
  return valid1 && (!valid0 || int32_t(seq1 - seq0) > 0) ? 1 : 0;
}
inline bool valid(const Header &h, size_t fileSize, const char *bytes) {
  return h.magic == Magic && h.length > 0 && h.length <= MaxLength &&
         fileSize == sizeof(Header) + h.length && crc32(bytes, h.length) == h.checksum;
}
} // namespace configjournal
