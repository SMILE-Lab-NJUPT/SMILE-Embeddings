#pragma once
#include <WString.h>
#include <ArduinoJson.h>
#include <FastLED.h>

namespace ws2812b {
    inline u8 BROADCAST_ADDRESS[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    constexpr u8 LED_PIN = 16;
    constexpr u8 LEDS_NUM = 10;
    inline JsonDocument doc;
    inline CRGB leds[LEDS_NUM];

    void setup();

    void loop();

    void parse_json(String json);

    void update(const JsonDocument &new_colors);
}
