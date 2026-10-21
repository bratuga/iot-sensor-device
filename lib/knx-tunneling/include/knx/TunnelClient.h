#pragma once
#include <stddef.h>
#include <stdint.h>

namespace knxip {
// IPv4 is stored in network order, e.g. 0xC000020A == 192.0.2.10.
struct Endpoint {
    uint32_t ip = 0;
    uint16_t port = 0;
    bool operator==(const Endpoint& other) const { return ip == other.ip && port == other.port; }
    bool operator!=(const Endpoint& other) const { return !(*this == other); }
};
constexpr uint16_t groupAddress(unsigned main, unsigned middle, unsigned sub) {
    return uint16_t((main << 11) | (middle << 8) | sub);
}
enum class AddressMode { Assigned, RequestV2, RequireV2 };
struct Config {
    Endpoint gateway;                         // mandatory, usually port 3671
    AddressMode addressMode = AddressMode::Assigned;
    uint16_t requestedAddress = 0;             // only sent with extended CRI
    bool strictAddress = false;                // reject a different assigned IA
    uint16_t maxTelegramsPerSecond = 5;         // 1..1000, pacing, no initial burst
    uint32_t heartbeatIntervalMs = 30000;
    uint32_t responseTimeoutMs = 10000;
    uint32_t reconnectDelayMs = 3000;
    uint32_t orphanCooldownMs = 130000;         // unconfirmed session cleanup
};
enum class State { Stopped, Connecting, Connected, Closing, Backoff, Blocked };
enum class Event {
    Connecting, Connected, Disconnected, Retrying, AddressMismatch,
    GatewayError, MalformedPacket, SendFailure, ConnectTimeout,
    HeartbeatTimeout, SendTimeout, SequenceError, QueueDiscarded, Stopped
};
enum class Result { Queued, NotConnected, QueueFull, InvalidArgument };
enum class Operation : uint16_t { Read = 0, Response = 0x40, Write = 0x80 };
struct Telegram {
    uint16_t source = 0;
    uint16_t group = 0;
    Operation operation = Operation::Read;
    bool compact = true;
    uint8_t size = 0;
    uint8_t data[14] = {};                     // copied; independent of UDP buffer
    bool asBool(bool& value) const {
        if (operation == Operation::Read || !compact || size != 1 || data[0] > 1) return false;
        value = data[0] != 0; return true;
    }
};
struct Stats {
    uint32_t received = 0, duplicates = 0, malformed = 0, ignored = 0;
    uint32_t sentAttempts = 0, acknowledged = 0, failed = 0, discarded = 0;
};
class Transport {
public:
    virtual ~Transport() = default;
    // Copy/send one datagram synchronously. Must not call back into the client.
    virtual bool send(const Endpoint&, const uint8_t*, size_t) = 0;
};
class Listener {
public:
    virtual ~Listener() = default;
    virtual void onTelegram(const Telegram&) {}
    virtual void onEvent(Event, uint16_t /*status or address*/) {}
    // token identifies the caller's queued request. Success means tunnel ACK,
    // not physical KNX delivery nor an actuator's resulting state.
    virtual void onSent(uint32_t /*token*/, bool /*acknowledged*/) {}
};
class TunnelClient {
public:
    static constexpr size_t QueueCapacity = 16;
    TunnelClient(Transport&, Listener&);
    bool start(const Config&, Endpoint local, uint32_t nowMs);
    void poll(uint32_t nowMs);
    void onDatagram(Endpoint peer, const uint8_t*, size_t, uint32_t nowMs);
    void stop(uint32_t nowMs);                  // graceful; keep polling until stopped
    void networkLost(uint32_t nowMs);           // datagrams no longer available
    Result writeBool(uint16_t group, bool value, uint32_t token = 0);
    Result write(uint16_t group, const uint8_t*, uint8_t size,
                 bool compact = false, uint32_t token = 0);
    Result respond(uint16_t group, const uint8_t*, uint8_t size,
                   bool compact = false, uint32_t token = 0);
    Result read(uint16_t group, uint32_t token = 0);
    State state() const { return state_; }
    uint16_t individualAddress() const { return assigned_; }
    Endpoint dataEndpoint() const { return data_; }
    size_t queued() const { return count_; }
    const Stats& stats() const { return stats_; }
    // Validates cEMI L_Data.ind standard group read/write/response.
    static bool decode(const uint8_t*, size_t, Telegram&);
private:
    struct Pending { Telegram telegram; uint32_t token; };
    enum class AfterClose { Retry, Stop, Block };
    Transport& transport_;
    Listener& listener_;
    Config config_{};
    Endpoint local_{}, data_{};
    State state_ = State::Stopped;
    AfterClose afterClose_ = AfterClose::Retry;
    Stats stats_{};
    Pending queue_[QueueCapacity]{};
    size_t head_ = 0, count_ = 0;
    uint16_t assigned_ = 0;
    uint8_t channel_ = 0, rxSeq_ = 0, txSeq_ = 0, txTries_ = 0, heartbeatTries_ = 0;
    bool rxSeen_ = false, txPending_ = false, heartbeatPending_ = false;
    bool extended_ = false, pacingActive_ = false, possibleOrphan_ = false;
    bool restartCooling_ = false;
    uint32_t restartNotBefore_ = 0;
    uint32_t deadline_ = 0, txDeadline_ = 0, heartbeatAt_ = 0, heartbeatDeadline_ = 0;
    uint32_t nextTelegram_ = 0, lastNow_ = 0;
    uint32_t spacing() const;
    void hpai(uint8_t*) const;
    void connect(uint32_t);
    bool sendControl(uint16_t);
    void heartbeat(uint32_t);
    void transmit(uint32_t);
    Result enqueue(Operation,uint16_t,const uint8_t*,uint8_t,bool,uint32_t);
    void discardQueue();
    void backoff(uint32_t,uint32_t);
    void close(AfterClose,uint32_t);
    void closed(uint32_t,bool);
    void event(Event e,uint16_t detail=0) { listener_.onEvent(e,detail); }
};
}
