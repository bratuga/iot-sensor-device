#pragma once
#include "SensorCore.h"
namespace device {
inline bool outputPin(int p) {
  switch (p) {
  case 4: case 13: case 14: case 15: case 16: case 17: case 18: case 19:
  case 21: case 22: case 23: case 25: case 26: case 27: case 32: case 33: return true;
  default: return false;
  }
}
inline bool relayLevel(bool on, bool activeLow) { return on != activeLow; }
// Coalesce commands while preserving a new feedback request during an outstanding ACK.
struct RelayState {
  bool on = false, dirty = true, pending = false, readRequested = false, queuedResponse = false;
  bool attempted = false;
  uint32_t revision = 0, queuedRevision = 0, attemptedAt = 0;
  void command(bool value) { on = value; dirty = true; ++revision; }
  void requestRead() { readRequested = true; }
  bool due(uint32_t now) const {
    return !pending && (dirty || readRequested) &&
           (!attempted || sensor::elapsed(now, attemptedAt, 250));
  }
  void queued(uint32_t now, bool response) {
    pending = true; queuedRevision = revision; attemptedAt = now; attempted = true;
    queuedResponse = response;
    if (response) readRequested = false;
  }
  void complete(bool ok) {
    if (!pending) return;
    pending = false;
    if (ok && revision == queuedRevision && !queuedResponse) dirty = false;
    if (!ok && queuedResponse) readRequested = true;
  }
  void reconnect() { pending = false; dirty = true; }
};
}
