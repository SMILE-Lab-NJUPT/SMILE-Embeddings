#include "ws2812b.h"

void ws2812b::ws2812b_init() {
    CFastLED::addLeds<NEOPIXEL, 12>(leds, NUM_LEDS);
    for (auto &led: leds)
        led.r = 0;
    FastLED.show();
}

void ws2812b::parse_json(String json) {
    Serial.println(json);
    if (const DeserializationError error = deserializeJson(doc, json)) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
    } else
        update(doc["data"]["color"]);
}

void ws2812b::update(const JsonDocument &new_colors) {
    for (u8 i = 0; i < new_colors.size() && i < NUM_LEDS; i++)
        leds[i] = new_colors[i].as<u32>();
    for (u8 i = new_colors.size(); i < NUM_LEDS; i++)
        leds[i] = CRGB::Black;
    FastLED.show();
}

void ws2812b::loop() {
    while (Serial.available()) {
        const String receivedLine = Serial.readStringUntil('\n');
        String json = R"({"device_name":"light1","MAC":"","data":{"color":[)";
        deserializeJson(doc, receivedLine);
        for (u32 i = 0; i < doc.size(); i++) {
            json += (doc[i][0].as<u32>() << 16) + (doc[i][1].as<u32>() << 8) + doc[i][2].as<u32>();
            json += ",";
        }
        if (doc.size())
            json.remove(json.length() - 1); // remove last comma
        json += "]}}";
        ws2812b::parse_json(json);
    }
}
