#pragma once
#include <WString.h>
#include <ArduinoJson.h>
#include <FastLED.h>

namespace ws2812b {
    inline JsonDocument doc;
    constexpr int NUM_LEDS = 10;
    inline CRGB leds[NUM_LEDS];

    void ws2812b_init();

    void parse_json(String json);

    void update(const JsonDocument& new_colors);

    void loop();
}
