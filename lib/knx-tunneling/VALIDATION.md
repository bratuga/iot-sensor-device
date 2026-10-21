# Validation record - 2026-10-01

The original ESP32 prototype was reported working by its owner with an existing
IP interface. This portable implementation is a separate refactoring. Its
validation is automated/simulated and compile-only unless stated otherwise.

## Automated runtime tests

- 171 assertions across 11 scenario suites passed on Windows with Zig/Clang 0.13.0.
- Undefined-behavior sanitizer enabled; module/test warnings treated as errors.
- 50,000 randomized/malformed datagrams, with exact buffers ending at an inaccessible
  memory page on Windows, exercise packet handling and detect parser over-reads.
- Group 0/1 forwarding with independent expected packet bytes; read, raw multibyte
  write/response; copied callback payloads; no feedback echo.
- Queue saturation/discard notifications; pacing at 1/2/5 telegrams/sec, no catch-up
  burst, retries counted toward pacing; controls bypass the rate gate.
- Lost/error ACK, two attempts with identical sequence, graceful disconnect,
  connect timeout/orphan cooldown, late response cleanup, server close.
- Extended-CRI fallback, strict IA mismatch, correct server-assigned source IA.
- Heartbeat retries/failure, network loss, restart after uncertain cleanup.
- Invalid peer/port/channel/length, duplicates, uint8 sequence and uint32 clock wrap.

## Practical limits

The same module compiled successfully for classic ESP32 with PlatformIO
espressif32 6.9.0 / Arduino 2.0.17. The compile-only fixture has no socket,
credentials, display or application logic; it was not flashed to a device.

No live group writes were performed during this review. The mock does not emulate
Wi-Fi, OS scheduling, the physical KNX line, gateway-specific filtering or every
firmware quirk. Long-running operation still needs target integration validation.
No credentials, firmware binaries or private installation profile belong in the
shareable bundle. The public package has a deterministic file allowlist.

## Import validation - 2026-10-21

The unchanged C++ sources passed the existing host test runner on Linux with
undefined-behavior sanitizer: 169 checks, 11 scenario suites and 50,000 malformed/
random datagrams. Windows-only guard-page checks were not exercised on this host.
ESP32 compilation and physical KNX operation were not repeated during import.

Only the module, build metadata, tests and integration documentation were imported.
Build outputs, compiler caches, PlatformIO state and the unrelated skill-packaging
script were excluded. The private installation notes were not committed.
