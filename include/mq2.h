#pragma once
#include <WString.h>
#include <ArduinoJson.h>

namespace mq2 {
    inline u8 BROADCAST_ADDRESS[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    constexpr u8 STATE_PIN = 5;
    inline JsonDocument doc;
    inline String json;

    void setup();

    void loop();
}
