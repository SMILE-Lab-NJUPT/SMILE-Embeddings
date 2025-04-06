#pragma once
#include <WString.h>
#include <ArduinoJson.h>

namespace ir_remote {
    inline u8 BROADCAST_ADDRESS[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    constexpr u8 LED_PIN = 4;
    inline JsonDocument doc;

    void setup();

    void loop();

    void parse_json(String json);

    void sent(const u8 *data1, const u8 *data2);
}
