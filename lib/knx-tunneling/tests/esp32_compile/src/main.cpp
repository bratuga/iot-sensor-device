// Compile-only portability check; never opens a socket or writes to the bus.
#include <Arduino.h>
#include <knx/TunnelClient.h>
struct Sink:knxip::Transport {
    bool send(const knxip::Endpoint&,const uint8_t*,size_t) override { return false; }
};
Sink transport;
knxip::Listener listener;
knxip::TunnelClient client(transport,listener);
void setup() {
    knxip::Config config;
    config.gateway={0xC0000214,3671};
    client.start(config,{0xC000020A,41000},millis());
}
void loop() { client.poll(millis()); delay(1); }
