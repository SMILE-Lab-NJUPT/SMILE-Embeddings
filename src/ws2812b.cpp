#include "ws2812b.h"
#include <ESP8266WiFi.h>
#include <espnow.h>

void ws2812b::setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    CFastLED::addLeds<NEOPIXEL, LED_PIN>(leds, LEDS_NUM);
    for (auto &led: leds)
        led = CRGB::Black;
    FastLED.show();

    if (esp_now_init()) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);

    esp_now_register_recv_cb([](u8 *, u8 *data, u8) {
        parse_json(String(reinterpret_cast<char *>(data)));
    });
}

void ws2812b::loop() {
    while (Serial.available()) {
        const String json = Serial.readStringUntil('\n');
        parse_json(json);
    }
}

void ws2812b::parse_json(String json) {
    if (const DeserializationError error = deserializeJson(doc, json)) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
    } else if (doc["device_name"].as<String>() == DEVICE_NAME) {
        Serial.println(json);
        update(doc["data"]["color"]);
    }
}

void ws2812b::update(const JsonDocument &new_colors) {
    for (u8 i = 0; i < new_colors.size() && i < LEDS_NUM; i++)
        leds[i] = new_colors[i].as<u32>();
    for (u8 i = new_colors.size(); i < LEDS_NUM; i++)
        leds[i] = CRGB::Black;
    FastLED.show();
}
