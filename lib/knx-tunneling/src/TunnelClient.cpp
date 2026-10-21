#include "knx/TunnelClient.h"
#include <string.h>

namespace knxip {
namespace {
uint16_t word(const uint8_t* p) { return uint16_t((uint16_t(p[0])<<8)|p[1]); }
void put(uint8_t* p,uint16_t v) { p[0]=uint8_t(v>>8); p[1]=uint8_t(v); }
void header(uint8_t* p,uint16_t service,uint16_t size) {
    p[0]=6; p[1]=0x10; put(p+2,service); put(p+4,size);
}
bool due(uint32_t now,uint32_t deadline) { return int32_t(now-deadline)>=0; }
bool validTime(uint32_t v) { return v>0 && v<0x40000000u; }
bool validIp(uint32_t ip) { return ip && (ip>>24)<224 && (ip>>24)!=127; }
}
TunnelClient::TunnelClient(Transport& t,Listener& l):transport_(t),listener_(l) {}
uint32_t TunnelClient::spacing() const { return (1000u+config_.maxTelegramsPerSecond-1)/config_.maxTelegramsPerSecond; }
bool TunnelClient::start(const Config& c,Endpoint local,uint32_t now) {
    if (state_!=State::Stopped || !validIp(c.gateway.ip) || !c.gateway.port ||
        !validIp(local.ip) || !local.port || c.maxTelegramsPerSecond<1 ||
        c.maxTelegramsPerSecond>1000 || !validTime(c.responseTimeoutMs) ||
        !validTime(c.heartbeatIntervalMs) || !validTime(c.reconnectDelayMs) ||
        !validTime(c.orphanCooldownMs) ||
        ((c.addressMode!=AddressMode::Assigned || c.strictAddress) && !c.requestedAddress)) return false;
    config_=c; local_=local; data_=c.gateway; stats_={}; head_=count_=0;
    assigned_=0; txSeq_=rxSeq_=0; rxSeen_=false; txPending_=heartbeatPending_=false;
    extended_=c.addressMode!=AddressMode::Assigned; possibleOrphan_=false;
    pacingActive_=false; lastNow_=now;
    if(restartCooling_ && !due(now,restartNotBefore_)) {
        state_=State::Backoff; deadline_=restartNotBefore_; event(Event::Retrying);
    } else { restartCooling_=false; connect(now); }
    return true;
}
void TunnelClient::hpai(uint8_t* p) const {
    p[0]=8; p[1]=1;
    p[2]=uint8_t(local_.ip>>24); p[3]=uint8_t(local_.ip>>16);
    p[4]=uint8_t(local_.ip>>8); p[5]=uint8_t(local_.ip); put(p+6,local_.port);
}
void TunnelClient::connect(uint32_t now) {
    uint8_t p[28]={}; const uint16_t n=extended_?28:26;
    header(p,0x0205,n); hpai(p+6); hpai(p+14);
    p[22]=extended_?6:4; p[23]=4; p[24]=2;
    if (extended_) put(p+26,config_.requestedAddress);
    state_=State::Connecting; deadline_=now+config_.responseTimeoutMs;
    if (!transport_.send(config_.gateway,p,n)) {
        event(Event::SendFailure); backoff(now,config_.reconnectDelayMs); return;
    }
    possibleOrphan_=true; event(Event::Connecting);
}
bool TunnelClient::sendControl(uint16_t service) {
    uint8_t p[16]={}; header(p,service,16); p[6]=channel_; hpai(p+8);
    return transport_.send(config_.gateway,p,16);
}
void TunnelClient::discardQueue() {
    const size_t previous=count_;
    // Callbacks cannot enqueue successfully once a disconnect begins.
    while(count_) {
        uint32_t token=queue_[head_].token;
        head_=(head_+1)%QueueCapacity; --count_; ++stats_.discarded;
        listener_.onSent(token,false);
    }
    txPending_=false; head_=0;
    if(previous) event(Event::QueueDiscarded,uint16_t(previous));
}
void TunnelClient::backoff(uint32_t now,uint32_t delay) {
    state_=State::Backoff; deadline_=now+delay;
    heartbeatPending_=false; discardQueue(); event(Event::Retrying);
}
void TunnelClient::close(AfterClose next,uint32_t now) {
    afterClose_=next; state_=State::Closing; deadline_=now+config_.responseTimeoutMs;
    heartbeatPending_=false; discardQueue();
    if(!sendControl(0x0209)) event(Event::SendFailure);
    event(Event::Disconnected);
}
void TunnelClient::closed(uint32_t now,bool confirmed) {
    possibleOrphan_=!confirmed; heartbeatPending_=false; txPending_=false;
    if(afterClose_==AfterClose::Stop) {
        restartCooling_=!confirmed;
        restartNotBefore_=now+config_.orphanCooldownMs;
        state_=State::Stopped; event(Event::Stopped);
    }
    else if(afterClose_==AfterClose::Block) state_=State::Blocked;
    else backoff(now,confirmed?config_.reconnectDelayMs:config_.orphanCooldownMs);
}
void TunnelClient::stop(uint32_t now) {
    lastNow_=now;
    if(state_==State::Connected) close(AfterClose::Stop,now);
    else if(state_==State::Closing) afterClose_=AfterClose::Stop;
    else {
        if(possibleOrphan_) { restartCooling_=true; restartNotBefore_=now+config_.orphanCooldownMs; }
        state_=State::Stopped; discardQueue(); event(Event::Stopped);
    }
}
void TunnelClient::networkLost(uint32_t now) {
    lastNow_=now;
    if(state_==State::Stopped || state_==State::Blocked) return;
    if(state_==State::Closing && afterClose_==AfterClose::Stop) {
        restartCooling_=true; restartNotBefore_=now+config_.orphanCooldownMs;
        state_=State::Stopped; discardQueue(); event(Event::Stopped); return;
    }
    // Adapter must wait until connectivity returns before polling again.
    possibleOrphan_=possibleOrphan_ || state_==State::Connected || state_==State::Closing;
    backoff(now,possibleOrphan_?config_.orphanCooldownMs:config_.reconnectDelayMs);
    event(Event::Disconnected);
}
Result TunnelClient::enqueue(Operation op,uint16_t group,const uint8_t* data,uint8_t size,bool compact,uint32_t token) {
    if(group>0x7FFF || (op!=Operation::Read && (!data || !size || size>14 || (compact && (size!=1 || data[0]>63))))) return Result::InvalidArgument;
    if(state_!=State::Connected) return Result::NotConnected;
    if(count_==QueueCapacity) return Result::QueueFull;
    Pending& p=queue_[(head_+count_)%QueueCapacity]; p={}; p.token=token;
    p.telegram.group=group; p.telegram.operation=op; p.telegram.compact=compact;
    p.telegram.size=op==Operation::Read?0:size;
    if(p.telegram.size) memcpy(p.telegram.data,data,size);
    ++count_; return Result::Queued;
}
Result TunnelClient::writeBool(uint16_t group,bool value,uint32_t token) {
    uint8_t v=value?1:0; return write(group,&v,1,true,token);
}
Result TunnelClient::write(uint16_t g,const uint8_t* p,uint8_t n,bool compact,uint32_t token) { return enqueue(Operation::Write,g,p,n,compact,token); }
Result TunnelClient::respond(uint16_t g,const uint8_t* p,uint8_t n,bool compact,uint32_t token) { return enqueue(Operation::Response,g,p,n,compact,token); }
Result TunnelClient::read(uint16_t g,uint32_t token) { return enqueue(Operation::Read,g,nullptr,0,true,token); }
void TunnelClient::transmit(uint32_t now) {
    const Telegram& t=queue_[head_].telegram;
    const bool embedded=t.compact || t.operation==Operation::Read;
    const uint16_t n=uint16_t(21+(embedded?0:t.size)); uint8_t p[35]={};
    header(p,0x0420,n); p[6]=4; p[7]=channel_; p[8]=txSeq_;
    p[10]=0x11; p[12]=0xBC; p[13]=0xE0; put(p+14,assigned_); put(p+16,t.group);
    p[18]=uint8_t(embedded?1:t.size+1); p[20]=uint8_t(t.operation);
    if(t.operation!=Operation::Read) {
        if(embedded) p[20]|=t.data[0]; else memcpy(p+21,t.data,t.size);
    }
    // Every attempt consumes a pacing slot, even if the transport reports failure.
    pacingActive_=true; nextTelegram_=now+spacing(); ++stats_.sentAttempts;
    txPending_=true; ++txTries_; txDeadline_=now+1000;
    if(!transport_.send(data_,p,n)) event(Event::SendFailure);
}
void TunnelClient::heartbeat(uint32_t now) {
    heartbeatPending_=true; ++heartbeatTries_; heartbeatDeadline_=now+config_.responseTimeoutMs;
    if(!sendControl(0x0207)) event(Event::SendFailure);
}
void TunnelClient::poll(uint32_t now) {
    lastNow_=now;
    if(state_==State::Backoff && due(now,deadline_)) { connect(now); return; }
    if(state_==State::Connecting && due(now,deadline_)) {
        // Do not create another potentially orphaned tunnel before cooldown.
        if(extended_ && config_.addressMode==AddressMode::RequestV2) extended_=false;
        event(Event::ConnectTimeout); backoff(now,config_.orphanCooldownMs); return;
    }
    if(state_==State::Closing && due(now,deadline_)) { closed(now,false); return; }
    if(state_!=State::Connected) return;
    if(heartbeatPending_ && due(now,heartbeatDeadline_)) {
        if(heartbeatTries_>=4) { event(Event::HeartbeatTimeout); close(AfterClose::Retry,now); return; }
        heartbeat(now);
    } else if(!heartbeatPending_ && due(now,heartbeatAt_)) { heartbeatTries_=0; heartbeat(now); }
    if(txPending_ && due(now,txDeadline_)) {
        if(txTries_>=2) { ++stats_.failed; event(Event::SendTimeout); close(AfterClose::Retry,now); return; }
        if(!pacingActive_ || due(now,nextTelegram_)) transmit(now);
    } else if(!txPending_ && count_ && (!pacingActive_ || due(now,nextTelegram_))) { txTries_=0; transmit(now); }
}
bool TunnelClient::decode(const uint8_t* p,size_t n,Telegram& t) {
    if(!p || n<2 || p[0]!=0x29) return false;
    const size_t c=2u+p[1];
    if(c+9>n || !(p[c]&0x80) || !(p[c+1]&0x80) || (p[c+1]&0x0F) || (p[c+7]&0xFC)) return false;
    const uint8_t length=p[c+6];
    if(length<1 || length>15 || c+8u+length!=n) return false;
    const uint16_t apci=uint16_t(((p[c+7]&3)<<8)|(p[c+8]&0xC0));
    if(apci!=0 && apci!=0x40 && apci!=0x80) return false;
    if(apci==0 && (length!=1 || (p[c+8]&0x3F))) return false;
    t={}; t.source=word(p+c+2); t.group=word(p+c+4);
    if(t.group>0x7FFF) return false;
    t.operation=static_cast<Operation>(apci); t.compact=length==1;
    if(!apci) return true;
    if(t.compact) { t.size=1; t.data[0]=p[c+8]&0x3F; }
    else { t.size=uint8_t(length-1); memcpy(t.data,p+c+9,t.size); }
    return true;
}
void TunnelClient::onDatagram(Endpoint peer,const uint8_t* p,size_t n,uint32_t now) {
    lastNow_=now;
    // UDP endpoints are checked before parsing; routing/other peers are ignored.
    if(peer!=config_.gateway && peer!=data_) { ++stats_.ignored; return; }
    if(!p || n<6 || n>512 || p[0]!=6 || p[1]!=0x10 || word(p+4)!=n) {
        ++stats_.malformed; event(Event::MalformedPacket); return;
    }
    const uint16_t service=word(p+2);
    if(service==0x0206 && peer==config_.gateway &&
       (state_==State::Connecting || (state_==State::Backoff && possibleOrphan_))) {
        if(n<8) { ++stats_.malformed; return; }
        if(p[7]) {
            if(state_!=State::Connecting) return;
            possibleOrphan_=false; event(Event::GatewayError,p[7]);
            if(extended_ && config_.addressMode==AddressMode::RequestV2 && (p[7]==0x22 || p[7]==0x23 || p[7]==0x29)) extended_=false;
            backoff(now,config_.reconnectDelayMs); return;
        }
        // Successful but malformed responses still allocate a channel: close it.
        channel_=p[6];
        if(n!=20 || p[8]!=8 || p[9]!=1 || p[16]!=4 || p[17]!=4) {
            ++stats_.malformed; event(Event::MalformedPacket); close(AfterClose::Retry,now); return;
        }
        assigned_=word(p+18);
        const uint32_t ip=(uint32_t(p[10])<<24)|(uint32_t(p[11])<<16)|(uint32_t(p[12])<<8)|p[13];
        const uint16_t port=word(p+14);
        // Route-back uses the source endpoint; otherwise only same gateway IP.
        if((ip && ip!=config_.gateway.ip) || (ip && !port) || !assigned_) {
            ++stats_.malformed; close(AfterClose::Retry,now); return;
        }
        data_={ip?ip:peer.ip,port?port:peer.port};
        if(state_==State::Backoff) { close(AfterClose::Retry,now); return; }
        if(config_.strictAddress && assigned_!=config_.requestedAddress) {
            event(Event::AddressMismatch,assigned_); close(AfterClose::Block,now); return;
        }
        state_=State::Connected; possibleOrphan_=true;
        rxSeq_=txSeq_=0; rxSeen_=txPending_=heartbeatPending_=false;
        heartbeatAt_=now+config_.heartbeatIntervalMs; event(Event::Connected,assigned_); return;
    }
    if(service==0x020A && state_==State::Closing && peer==config_.gateway) {
        if(n==8 && p[6]==channel_ && p[7]==0) closed(now,true);
        else if(n!=8) ++stats_.malformed;
        return;
    }
    if(service==0x0209 && (state_==State::Connected || state_==State::Closing) && peer==config_.gateway) {
        if(n!=16 || p[6]!=channel_ || p[7]!=0 || p[8]!=8 || p[9]!=1) { ++stats_.ignored; return; }
        uint8_t ack[8]={}; header(ack,0x020A,8); ack[6]=channel_;
        transport_.send(peer,ack,8);
        if(state_==State::Connected) { afterClose_=AfterClose::Retry; state_=State::Closing; discardQueue(); event(Event::Disconnected); }
        closed(now,true); return;
    }
    if(state_!=State::Connected) { ++stats_.ignored; return; }
    if(service==0x0208 && peer==config_.gateway) {
        if(n!=8 || p[6]!=channel_ || !heartbeatPending_) { ++stats_.ignored; return; }
        if(p[7]) { event(Event::GatewayError,p[7]); close(AfterClose::Retry,now); return; }
        heartbeatPending_=false; heartbeatAt_=now+config_.heartbeatIntervalMs; return;
    }
    if(service==0x0421 && peer==data_) {
        if(n!=10 || p[6]!=4 || p[7]!=channel_ || p[8]!=txSeq_ || !txPending_) { ++stats_.ignored; return; }
        if(p[9]) {
            event(Event::GatewayError,p[9]);
            if(txTries_>=2) { ++stats_.failed; close(AfterClose::Retry,now); }
            else txDeadline_=now;
            return;
        }
        uint32_t token=queue_[head_].token;
        head_=(head_+1)%QueueCapacity; --count_; ++txSeq_; txPending_=false;
        ++stats_.acknowledged; listener_.onSent(token,true); return;
    }
    if(service!=0x0420 || peer!=data_) { ++stats_.ignored; return; }
    if(n<12 || p[6]!=4 || p[7]!=channel_ || p[9]!=0) { ++stats_.malformed; return; }
    const bool fresh=p[8]==rxSeq_;
    const bool duplicate=rxSeen_ && p[8]==uint8_t(rxSeq_-1);
    uint8_t ack[10]={}; header(ack,0x0421,10); ack[6]=4; ack[7]=channel_; ack[8]=p[8];
    ack[9]=(fresh||duplicate)?0:4;
    if(!transport_.send(data_,ack,10)) event(Event::SendFailure);
    if(duplicate) { ++stats_.duplicates; return; }
    if(!fresh) { event(Event::SequenceError); return; }
    ++rxSeq_; rxSeen_=true;
    // Confirm the transport even for non-group cEMI (e.g. L_Data.con).
    Telegram telegram;
    if(decode(p+10,n-10,telegram)) { ++stats_.received; listener_.onTelegram(telegram); }
    else ++stats_.ignored;
}
}
