# Portable KNXnet/IP UDP tunneling client

This C++14 module handles a single KNXnet/IP UDP tunnel. It does not depend on
Arduino, Wi-Fi, ESP32, an OLED, a particular group-address map, or credentials.
The application supplies a datagram transport, a monotonic millisecond clock,
and callbacks. No heap allocations occur in the module itself.

## Files and integration

Compile `src/TunnelClient.cpp` and add `include/` to the include path. PlatformIO
can use this directory as a local library; its `library.json` is included.

```cpp
#include <knx/TunnelClient.h>
using namespace knxip;

// Implement Transport::send() using your UDP socket.
// Implement Listener::onTelegram(), onEvent(), onSent() as needed.
TunnelClient client(transport, listener);
Config config;
config.gateway = {0xC0000214, 3671}; // example: 192.0.2.20
config.addressMode = AddressMode::Assigned;
config.maxTelegramsPerSecond = 5;
client.start(config, {0xC000020A, 41000}, nowMs);
// Feed complete UDP datagrams with source IP and source port:
client.onDatagram(peer, bytes, length, nowMs);
client.poll(nowMs);
```

The sample network addresses are documentation-only and must be replaced.
Bind the UDP socket before `start`; use its actual local IP and port. IP values
are host integers representing network-order IPv4 bytes (192.0.2.20 = 0xC0000214).
The UDP adapter must copy datagrams or finish sending before `send` returns.
Pass the full datagram length; do not disguise truncated packets as complete.
Call `poll` frequently while the network is available, typically every 1-10 ms.

The client is single-threaded. Feed UDP and `poll` in the same task, or serialize
access. Callbacks can enqueue messages; they must be short, non-blocking and
must not call lifecycle functions (`start`, `stop`, `networkLost`, `poll`,
`onDatagram`) reentrantly. The Telegram passed to the callback owns its data,
but its reference is valid only during the callback; copy it to retain it.

## Parameters

| Parameter | Default | Meaning |
|---|---:|---|
| gateway | required | IPv4 and control port, normally UDP 3671 |
| addressMode | Assigned | v1 server-assigned IA; RequestV2 optionally falls back; RequireV2 stays extended |
| requestedAddress | 0 | IA encoded as area/line/device, only requested by extended CRI |
| strictAddress | false | Close and block if the returned IA differs from requestedAddress |
| maxTelegramsPerSecond | 5 | Pacing of outgoing group requests, including transport retransmissions |
| heartbeatIntervalMs | 30000 | Interval between successful heartbeat cycles |
| responseTimeoutMs | 10000 | Connect/heartbeat/disconnect response deadline |
| reconnectDelayMs | 3000 | Delay after rejection or confirmed session cleanup |
| orphanCooldownMs | 130000 | Delay after a potentially orphaned session; adapt to server timeout |

Rate input accepts 1..1000 for millisecond scheduling. This is an API bound,
not an assurance that the physical bus supports that rate. Choose a conservative
limit for the actual line; 5 is the starter default, not a universal capacity.
Rate applies to this instance only: multiple clients need a shared budget.
The spacing is ceil(1000/rate) ms, so some settings produce a slightly lower
effective rate. No initial burst and no catch-up burst after a delayed `poll`.
ACKs, heartbeats, connect and disconnect are not group telegrams and bypass
the pacing gate. Slow receivers, queue saturation or transport loss return
explicit results/events; nothing waits synchronously for space.

Tunneling v1 cannot select an arbitrary individual address. Use the returned
`individualAddress()` as the source of outgoing telegrams. Never open multiple
sessions to occupy slots until a preferred address is allocated. `RequestV2`
falls back only for unsupported CRI/type/layer or an unanswered request after
cooldown; busy/address-in-use responses do not cause v1 fallback.

## Group messaging

- `writeBool(group, value, token)` encodes compact DPT 1.
- `write(group, bytes, size, compact, token)` sends GroupValueWrite.
- `respond(...)` sends GroupValueResponse, for applications acting as a value source.
- `read(group, token)` sends GroupValueRead; an ACK is not the value response.
- Return value: `Queued`, `NotConnected`, `QueueFull`, `InvalidArgument`.
- Queue: 16 copied messages; compact payload is one 6-bit value, noncompact is
  1..14 raw bytes in standard frames. Payload byte ordering/DPT encoding belongs
  to the application.

Received group telegrams carry source IA, group, Read/Write/Response operation,
compact/raw data and payload size. `asBool` accepts only compact 0/1 writes or
responses. KNX telegrams do not include their DPT. Other DPT conversions must
use the configured group-address schema; do not guess a temperature from bytes.
The module receives only what the interface forwards. Individual management
frames and cEMI confirmations are transport-ACKed but not emitted as groups.

Feedback mappings belong to the caller, for example enqueueing a bool write
to group B on receiving group A. Avoid echo loops; do not map a group to itself
without explicit suppression. Tunnel duplicates are ACKed once more and do
not invoke the application twice. Separate, legitimate identical values still
invoke the callback. A token completes once: true means tunnel ACK, false
means discarded/unconfirmed. Delivery is not exactly-once across reconnects.

## Recovery and scope

The client uses the data endpoint returned by the gateway (which may use a
different UDP port). Zero-address route-back resolves to the sender endpoint;
different gateway IPs are rejected. Outgoing group requests wait for an ACK;
one retransmission uses the same sequence after 1 second and still respects
pacing. A second unsuccessful attempt closes the session. Heartbeats tolerate
three retries. Incoming sequence duplicates and wraparound are supported.

`stop(now)` initiates a graceful disconnect; keep feeding UDP and polling until
Stopped. If the network disappears, call `networkLost(now)` once, suspend `poll`
until it returns, and resume only with the same bound socket/IP. If either the
local IP or UDP port changes, create a fresh client after old-session cleanup.
Restarting the same client after an uncertain stop retains the cooldown. Do not
immediately recreate a different client: allow the gateway's orphan-session
timeout. Queued values are discarded on connection
loss to avoid replaying stale application commands; the caller gets failure
callbacks and decides what, if anything, to resubmit.

This module implements UDP link-layer tunneling and standard group telegrams.
It does not implement KNX Secure, TCP tunneling, routing, a TP electrical
interface, ETS product creation or a certified KNX stack.

## Tests

```text
python scripts/run_tests.py --compiler /path/to/clang++
# or g++, or zig (the runner adds "c++")
```

Tests use a mock datagram transport and deterministic clock, with independent
packet fixtures and randomized malformed input. They never contact a gateway
or attached device. See `VALIDATION.md` for results and their limits.

Authoritative protocol references used for review:
- https://github.com/XKNX/xknx/blob/master/xknx/knxip/connect_request.py
- https://github.com/XKNX/xknx/blob/master/xknx/knxip/connect_response.py
- https://github.com/XKNX/xknx/blob/master/xknx/io/tunnel.py
- https://github.com/XKNX/xknx/blob/master/xknx/knxip/error_code.py

Implementation is original to this project; referenced sources document wire
behavior and were not copied into the module. No network credentials or binary
firmware are included in the shareable package.
