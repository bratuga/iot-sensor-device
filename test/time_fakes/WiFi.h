#pragma once
extern bool fakeConnected;
struct WifiFake{bool isConnected(){return fakeConnected;}};
extern WifiFake WiFi;
