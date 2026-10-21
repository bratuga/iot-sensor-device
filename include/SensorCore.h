#pragma once
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
namespace sensor {
inline bool elapsed(uint32_t now, uint32_t then, uint32_t interval) {
  return uint32_t(now - then) >= interval;
}
inline bool parseGroup(const char *s, uint16_t &out) {
  if (!s || !*s)
    return false;
  unsigned parts[3] = {};
  for (unsigned i = 0; i < 3; ++i) {
    unsigned digits = 0;
    while (*s >= '0' && *s <= '9') {
      if (++digits > 3)
        return false;
      parts[i] = parts[i] * 10 + unsigned(*s++ - '0');
    }
    if (!digits)
      return false;
    if (i < 2) {
      if (*s++ != '/')
        return false;
    } else if (*s)
      return false;
  }
  if (parts[0] > 15 || parts[1] > 7 || parts[2] > 255)
    return false;
  out = uint16_t((parts[0] << 11) | (parts[1] << 8) | parts[2]);
  return out != 0;
}

inline bool encodeDpt9(float value, uint8_t out[2]) {
  if (!std::isfinite(value) || value < -671088.64f || value > 670760.96f)
    return false;
  double scaled = double(value) * 100.0;
  int exp = 0;
  long m = std::lround(scaled);
  while ((m < -2048 || m > 2047) && exp < 15) {
    ++exp;
    m = std::lround(scaled / std::pow(2.0, exp));
  }
  if (m < -2048 || m > 2047)
    return false;
  uint16_t raw = uint16_t((m < 0 ? 0x8000 : 0) | (exp << 11) | (uint16_t(m) & 0x7ff));
  out[0] = uint8_t(raw >> 8);
  out[1] = uint8_t(raw);
  return true;
}
inline uint8_t crc8(const uint8_t *p, unsigned n) {
  uint8_t crc = 0;
  while (n--) {
    uint8_t b = *p++;
    for (unsigned i = 0; i < 8; ++i) {
      bool mix = (crc ^ b) & 1;
      crc >>= 1;
      if (mix)
        crc ^= 0x8c;
      b >>= 1;
    }
  }
  return crc;
}
inline bool parseRom(const char *text, uint8_t out[8]) {
  if (!text || std::strlen(text) != 16)
    return false;
  for (unsigned i = 0; i < 8; ++i) {
    unsigned b = 0;
    for (unsigned j = 0; j < 2; ++j) {
      char c = text[2 * i + j];
      unsigned d;
      if (c >= '0' && c <= '9')
        d = c - '0';
      else if (c >= 'a' && c <= 'f')
        d = c - 'a' + 10;
      else if (c >= 'A' && c <= 'F')
        d = c - 'A' + 10;
      else
        return false;
      b = (b << 4) | d;
    }
    out[i] = uint8_t(b);
  }
  return out[0] == 0x28 && crc8(out, 7) == out[7];
}
struct Policy {
  uint32_t periodMs = 60000, minGapMs = 5000;
  float delta = 0.2f;
  bool periodic = true, onChange = true;
};
struct Publication {
  bool acknowledged = false, pending = false;
  float sentValue = 0, queuedValue = 0;
  uint32_t sentAt = 0, attemptAt = 0;
  bool attempted = false;
  bool due(float value, uint32_t now, const Policy &p) const {
    if (pending || !std::isfinite(value) || (attempted && !elapsed(now, attemptAt, p.minGapMs)))
      return false;
    if (!p.periodic && !p.onChange)
      return false;
    return !acknowledged || (p.periodic && elapsed(now, sentAt, p.periodMs)) ||
           (p.onChange && std::fabs(value - sentValue) >= p.delta);
  }
  void queued(float value, uint32_t now) {
    queuedValue = value;
    attemptAt = now;
    attempted = true;
    pending = true;
  }
  void complete(bool ok, uint32_t now) {
    if (!pending)
      return;
    pending = false;
    if (ok) {
      acknowledged = true;
      sentValue = queuedValue;
      sentAt = now;
    }
  }
  void reconnect() {
    acknowledged = false;
    pending = false;
  }
};
} // namespace sensor
