#pragma once
#include <WString.h>
#include <ArduinoJson.h>
#include <DHT.h>

namespace dht11 {
    inline u8 BROADCAST_ADDRESS[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    constexpr u8 DATA_PIN = 4;
    inline JsonDocument doc;
    inline String json;
    inline DHT dht;

    void setup();

    void loop();
}
