#include "knx/TunnelClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <utility>
#include <stdint.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using namespace knxip;
int checks=0;
#define CHECK(x) do { ++checks; if(!(x)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x); exit(1); } } while(0)
uint16_t word(const uint8_t* p) { return uint16_t((p[0]<<8)|p[1]); }
void put(std::vector<uint8_t>& p,size_t offset,uint16_t value) { p[offset]=uint8_t(value>>8); p[offset+1]=uint8_t(value); }
std::vector<uint8_t> packet(uint16_t service,size_t n) {
    std::vector<uint8_t> p(n); p[0]=6; p[1]=0x10; put(p,2,service); put(p,4,uint16_t(n)); return p;
}
struct Sent { Endpoint peer; std::vector<uint8_t> bytes; };
struct FakeTransport:Transport {
    std::vector<Sent> packets; bool succeeds=true;
    bool send(const Endpoint& peer,const uint8_t* p,size_t n) override {
        packets.push_back({peer,std::vector<uint8_t>(p,p+n)}); return succeeds;
    }
    size_t count(uint16_t type) const {
        size_t n=0; for(const auto& p:packets) if(word(p.bytes.data()+2)==type) ++n; return n;
    }
    Sent last(uint16_t type) const {
        for(auto i=packets.rbegin();i!=packets.rend();++i) if(word(i->bytes.data()+2)==type) return *i;
        fprintf(stderr,"Missing packet %x\n",type); exit(1);
    }
};
struct Observer:Listener {
    std::vector<Telegram> telegrams;
    std::vector<std::pair<Event,uint16_t>> events;
    std::vector<std::pair<uint32_t,bool>> completions;
    TunnelClient* client=nullptr; bool echo=false;
    void onTelegram(const Telegram& t) override {
        telegrams.push_back(t);
        bool bit;
        if(echo && t.group==groupAddress(7,0,0) && t.asBool(bit)) CHECK(client->writeBool(groupAddress(7,1,0),bit,42)==Result::Queued);
    }
    void onEvent(Event e,uint16_t d) override { events.push_back({e,d}); }
    void onSent(uint32_t token,bool success) override { completions.push_back({token,success}); }
};
struct Fixture {
    FakeTransport transport; Observer observer; TunnelClient client;
    Config config;
    Endpoint local{0xC000020A,41000}, data{0xC0000214,50000};
    Fixture():client(transport,observer) { config.gateway={data.ip,3671}; observer.client=&client; }
    void inject(const std::vector<uint8_t>& p,uint32_t now,Endpoint peer) { client.onDatagram(peer,p.data(),p.size(),now); }
    void connect(uint32_t start=0,uint32_t response=1,uint16_t address=0x11F2) {
        CHECK(client.start(config,local,start));
        auto p=packet(0x0206,20); p[6]=7; p[8]=8; p[9]=1;
        p[10]=uint8_t(data.ip>>24); p[11]=uint8_t(data.ip>>16); p[12]=uint8_t(data.ip>>8); p[13]=uint8_t(data.ip);
        put(p,14,data.port); p[16]=4; p[17]=4; put(p,18,address);
        inject(p,response,config.gateway); CHECK(client.state()==State::Connected);
    }
    void ack(uint8_t sequence,uint32_t now,uint8_t status=0) {
        auto p=packet(0x0421,10); p[6]=4; p[7]=7; p[8]=sequence; p[9]=status; inject(p,now,data);
    }
    void closed(uint32_t now) {
        auto p=packet(0x020A,8); p[6]=7; inject(p,now,config.gateway);
    }
    std::vector<uint8_t> incoming(uint8_t sequence,uint16_t group,uint8_t apdu=0x81) {
        auto p=packet(0x0420,21); p[6]=4; p[7]=7; p[8]=sequence;
        p[10]=0x29; p[12]=0xBC; p[13]=0xE0; put(p,14,0x1101); put(p,16,group); p[18]=1; p[20]=apdu; return p;
    }
};
void basicAndFeedback() {
    Fixture f; f.connect();
    auto request=f.transport.last(0x0205).bytes;
    CHECK(request.size()==26); CHECK(request[22]==4); CHECK(word(request.data()+12)==41000);
    CHECK(f.client.individualAddress()==0x11F2); CHECK(f.client.dataEndpoint()==f.data);
    f.observer.echo=true;
    auto p=f.incoming(0,groupAddress(7,0,0)); f.inject(p,2,f.data); f.client.poll(2);
    CHECK(f.observer.telegrams.size()==1); CHECK(f.transport.count(0x0421)==1);
    auto sent=f.transport.last(0x0420);
    const uint8_t expected[]={6,0x10,4,0x20,0,21,4,7,0,0,0x11,0,0xBC,0xE0,0x11,0xF2,0x39,0,1,0,0x81};
    CHECK(sent.peer==f.data); CHECK(sent.bytes.size()==sizeof(expected)); CHECK(memcmp(sent.bytes.data(),expected,sizeof(expected))==0);
    f.ack(0,3); CHECK(f.client.stats().acknowledged==1); CHECK(f.observer.completions.back()==std::make_pair(uint32_t(42),true));
    f.inject(p,4,f.data); CHECK(f.observer.telegrams.size()==1); CHECK(f.client.stats().duplicates==1);
    p=f.incoming(1,groupAddress(7,0,0),0x80); f.inject(p,5,f.data); f.client.poll(202);
    CHECK(f.transport.last(0x0420).bytes[20]==0x80); f.ack(1,203);
    CHECK(f.observer.telegrams[0].data[0]==1); // copied telegram survives later packets
    p=f.incoming(2,groupAddress(7,1,0)); f.inject(p,204,f.data); CHECK(f.client.queued()==0);
    p=f.incoming(3,groupAddress(7,0,0),0); f.inject(p,205,f.data); CHECK(f.client.queued()==0);
}
void rateAndQueue() {
    Fixture f; f.config.maxTelegramsPerSecond=2; f.connect();
    for(unsigned i=0;i<TunnelClient::QueueCapacity;++i) CHECK(f.client.writeBool(1,i&1,i)==Result::Queued);
    CHECK(f.client.writeBool(1,true)==Result::QueueFull);
    f.client.poll(10); f.ack(0,11);
    f.client.poll(509); CHECK(f.transport.count(0x0420)==1);
    f.client.poll(510); CHECK(f.transport.count(0x0420)==2); f.ack(1,511);
    f.client.poll(5000); CHECK(f.transport.count(0x0420)==3); // no catch-up burst
    f.ack(2,5001); f.client.poll(5001); CHECK(f.transport.count(0x0420)==3);
    f.client.poll(5499); CHECK(f.transport.count(0x0420)==3);
    f.client.poll(5500); CHECK(f.transport.count(0x0420)==4);
    f.client.stop(5501); CHECK(f.client.queued()==0); CHECK(f.client.stats().discarded==13);
    f.closed(5502); CHECK(f.client.state()==State::Stopped);
}
void retryAndControls() {
    Fixture f; f.config.maxTelegramsPerSecond=1; f.config.heartbeatIntervalMs=500; f.connect();
    CHECK(f.client.writeBool(1,true)==Result::Queued); f.client.poll(10);
    f.client.poll(501); CHECK(f.transport.count(0x0207)==1); CHECK(f.transport.count(0x0420)==1);
    f.client.poll(1009); CHECK(f.transport.count(0x0420)==1);
    f.client.poll(1010); CHECK(f.transport.count(0x0420)==2);
    CHECK(f.transport.last(0x0420).bytes[8]==0); // identical sequence on retry
    f.client.poll(2010); CHECK(f.client.state()==State::Closing);
    CHECK(f.transport.count(0x0209)==1); CHECK(f.client.stats().failed==1);
    f.closed(2011); CHECK(f.client.state()==State::Backoff);
    f.client.poll(5010); CHECK(f.transport.count(0x0205)==1);
    f.client.poll(5011); CHECK(f.transport.count(0x0205)==2);
}
void extendedAndAddress() {
    Fixture f; f.config.addressMode=AddressMode::RequestV2; f.config.requestedAddress=0x11F3;
    CHECK(f.client.start(f.config,f.local,0)); CHECK(f.transport.last(0x0205).bytes.size()==28);
    CHECK(word(f.transport.last(0x0205).bytes.data()+26)==0x11F3);
    auto error=packet(0x0206,8); error[7]=0x23; f.inject(error,1,f.config.gateway);
    f.client.poll(3001); CHECK(f.transport.last(0x0205).bytes.size()==26);
    Fixture strict; strict.config.requestedAddress=0x11F3; strict.config.strictAddress=true;
    CHECK(strict.client.start(strict.config,strict.local,0));
    auto p=packet(0x0206,20); p[6]=7; p[8]=8; p[9]=1; p[10]=192; p[12]=2; p[13]=20;
    put(p,14,50000); p[16]=4; p[17]=4; put(p,18,0x11F2);
    strict.inject(p,1,strict.config.gateway); CHECK(strict.client.state()==State::Closing);
    strict.closed(2); CHECK(strict.client.state()==State::Blocked);
    strict.client.poll(200000); CHECK(strict.transport.count(0x0205)==1);
}
void peersMalformedAndSequences() {
    Fixture f; f.connect(); auto p=f.incoming(0,1);
    f.inject(p,2,{f.data.ip,50001}); CHECK(f.observer.telegrams.empty()); CHECK(f.transport.count(0x0421)==0);
    p[7]=8; f.inject(p,3,f.data); CHECK(f.observer.telegrams.empty());
    p[7]=7; p[5]=20; f.inject(p,4,f.data); CHECK(f.client.stats().malformed>=1);
    p[5]=21; p[8]=255; f.inject(p,5,f.data); CHECK(f.transport.last(0x0421).bytes[9]==4);
    CHECK(f.observer.telegrams.empty());
    for(unsigned i=0;i<258;++i) { p=f.incoming(uint8_t(i),1); f.inject(p,10+i,f.data); }
    CHECK(f.observer.telegrams.size()==258); CHECK(f.transport.last(0x0421).bytes[9]==0);
    f.inject(p,269,f.data); CHECK(f.observer.telegrams.size()==258);
}
void heartbeatAndLoss() {
    Fixture f; f.connect(); f.client.poll(30001); CHECK(f.transport.count(0x0207)==1);
    auto p=packet(0x0208,8); p[6]=7;
    f.inject(p,30002,f.config.gateway); f.client.poll(60001); CHECK(f.transport.count(0x0207)==1);
    f.client.poll(60002); CHECK(f.transport.count(0x0207)==2);
    f.client.poll(70002); f.client.poll(80002); f.client.poll(90002);
    CHECK(f.transport.count(0x0207)==5); f.client.poll(100002); CHECK(f.client.state()==State::Closing);
    Fixture lost; lost.connect(); CHECK(lost.client.writeBool(1,true,7)==Result::Queued);
    lost.client.networkLost(2); CHECK(lost.client.queued()==0); CHECK(!lost.observer.completions.back().second);
    lost.client.poll(130001); CHECK(lost.transport.count(0x0205)==1);
    lost.client.poll(130002); CHECK(lost.transport.count(0x0205)==2);
}
void timeoutLateAndServerClose() {
    Fixture f; CHECK(f.client.start(f.config,f.local,0)); f.client.poll(10000);
    CHECK(f.client.state()==State::Backoff); f.client.poll(139999); CHECK(f.transport.count(0x0205)==1);
    f.client.poll(140000); CHECK(f.transport.count(0x0205)==2);
    Fixture late; CHECK(late.client.start(late.config,late.local,0)); late.client.poll(10000);
    auto p=packet(0x0206,20); p[6]=7; p[8]=8; p[9]=1; p[16]=4; p[17]=4; put(p,18,0x11F2);
    late.inject(p,10001,late.config.gateway); CHECK(late.client.state()==State::Closing);
    late.closed(10002); CHECK(late.client.state()==State::Backoff);
    Fixture server; server.connect(); p=packet(0x0209,16); p[6]=7; p[8]=8; p[9]=1;
    server.inject(p,2,server.config.gateway); CHECK(server.transport.count(0x020A)==1); CHECK(server.client.state()==State::Backoff);
}
void payloadsAndWrap() {
    Fixture f; f.connect(0xFFFFFF00u,0xFFFFFF01u); uint8_t bytes[]={7,0xD0};
    CHECK(f.client.write(2,bytes,2,false,1)==Result::Queued); f.client.poll(0xFFFFFFF0u);
    auto p=f.transport.last(0x0420).bytes; CHECK(p.size()==23); CHECK(p[18]==3 && p[20]==0x80 && p[21]==7 && p[22]==0xD0);
    f.ack(0,0xFFFFFFF1u); CHECK(f.client.read(2)==Result::Queued);
    f.client.poll(0xB7); CHECK(f.transport.count(0x0420)==1);
    f.client.poll(0xB8); CHECK(f.transport.count(0x0420)==2);
    p=f.transport.last(0x0420).bytes; CHECK(p.size()==21 && p[20]==0); f.ack(1,0xB9);
    CHECK(f.client.respond(2,bytes,2)==Result::Queued); f.client.poll(0x180);
    CHECK(f.transport.last(0x0420).bytes[20]==0x40);
    CHECK(f.client.write(0x8000,bytes,2)==Result::InvalidArgument);
    CHECK(f.client.write(2,nullptr,2)==Result::InvalidArgument);
    CHECK(f.client.write(2,bytes,15)==Result::InvalidArgument);
    CHECK(f.client.write(2,bytes,2,true)==Result::InvalidArgument);
}
void decoderAndFuzz() {
    uint8_t p[]={0x29,3,1,1,0x55,0xBC,0xE0,0x11,1,0x38,0,1,0,0x41}; Telegram t;
    CHECK(TunnelClient::decode(p,sizeof(p),t)); bool bit=false; CHECK(t.asBool(bit) && bit);
    for(size_t n=0;n<sizeof(p);++n) CHECK(!TunnelClient::decode(p,n,t));
    uint8_t raw[]={0x29,0,0xBC,0xE0,0x11,1,0x09,2,3,0,0x80,7,0xD0};
    CHECK(TunnelClient::decode(raw,sizeof(raw),t)); CHECK(!t.compact && t.size==2 && t.data[1]==0xD0);
    uint32_t random=0x72A801;
    Fixture f; f.connect();
#ifdef _WIN32
    SYSTEM_INFO system; GetSystemInfo(&system); const size_t pageSize=system.dwPageSize;
    uint8_t* guard=static_cast<uint8_t*>(VirtualAlloc(nullptr,pageSize*2,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));
    CHECK(guard!=nullptr); DWORD previous;
    CHECK(VirtualProtect(guard+pageSize,pageSize,PAGE_NOACCESS,&previous));
#endif
    for(unsigned i=0;i<50000;++i) {
        random=random*1664525u+1013904223u; size_t n=random%80;
        std::vector<uint8_t> bytes(n);
        for(auto& b:bytes) { random=random*1664525u+1013904223u; b=uint8_t(random>>24); }
        if(i%2==0 && n>=10) { bytes[0]=6; bytes[1]=0x10; put(bytes,2,0x0420); put(bytes,4,uint16_t(n)); bytes[6]=4; bytes[7]=7; }
        const uint8_t* data=bytes.data();
#ifdef _WIN32
        // Put the complete datagram directly before an inaccessible page.
        // Any parser over-read crashes the test instead of hiding in vector capacity.
        uint8_t* exact=guard+pageSize-n; if(n) memcpy(exact,bytes.data(),n); data=exact;
#endif
        TunnelClient::decode(data,bytes.size(),t);
        f.client.onDatagram(f.data,data,bytes.size(),2+i);
    }
#ifdef _WIN32
    VirtualFree(guard,0,MEM_RELEASE);
#endif
    CHECK(f.client.state()==State::Connected);
}
void restartAfterUncertainClose() {
    Fixture f; f.connect(); f.client.stop(2); f.client.poll(10002);
    CHECK(f.client.state()==State::Stopped);
    CHECK(f.client.start(f.config,f.local,10003)); CHECK(f.client.state()==State::Backoff);
    CHECK(f.transport.count(0x0205)==1); f.client.poll(140002); CHECK(f.transport.count(0x0205)==2);
    Fixture lost; lost.connect(); lost.client.stop(2); lost.client.networkLost(3);
    CHECK(lost.client.state()==State::Stopped); lost.client.poll(200000);
    CHECK(lost.transport.count(0x0205)==1);
    Fixture pending; CHECK(pending.client.start(pending.config,pending.local,0)); pending.client.stop(1);
    CHECK(pending.client.start(pending.config,pending.local,2)); CHECK(pending.client.state()==State::Backoff);
}
void configAndSendFailures() {
    Fixture f; f.config.maxTelegramsPerSecond=0; CHECK(!f.client.start(f.config,f.local,0));
    f.config.maxTelegramsPerSecond=1001; CHECK(!f.client.start(f.config,f.local,0));
    f.config.maxTelegramsPerSecond=5; f.config.gateway.ip=0; CHECK(!f.client.start(f.config,f.local,0));
    Fixture fail; fail.transport.succeeds=false; CHECK(fail.client.start(fail.config,fail.local,0));
    CHECK(fail.client.state()==State::Backoff); fail.client.poll(3000); CHECK(fail.transport.count(0x0205)==2);
    Fixture ackFail; ackFail.connect(); ackFail.client.writeBool(1,true); ackFail.client.poll(2);
    ackFail.ack(0,3,4); ackFail.client.poll(201); CHECK(ackFail.transport.count(0x0420)==1);
    ackFail.client.poll(202); CHECK(ackFail.transport.count(0x0420)==2);
    ackFail.ack(0,203,4); CHECK(ackFail.client.state()==State::Closing);
}
int main() {
    basicAndFeedback(); rateAndQueue(); retryAndControls(); extendedAndAddress();
    peersMalformedAndSequences(); heartbeatAndLoss(); timeoutLateAndServerClose();
    payloadsAndWrap(); decoderAndFuzz(); configAndSendFailures();
    restartAfterUncertainClose();
    printf("PASS: %d checks, 11 scenario suites, 50000 guarded malformed/random datagrams\n",checks);
}
