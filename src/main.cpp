#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ArduinoJson.h>

#include "ws2812b.h"

u8 broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

constexpr u8 DEVICE_NAME[] = "light1";
u8 device_type = 0;

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    switch (device_type) {
        case 0:
            ws2812b::ws2812b_init();
            break;
        default: return;
    }

    if (esp_now_init()) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_register_send_cb([](u8 *, const u8 sendStatus) {
        Serial.println("Send broadcast status: " + String(sendStatus));
    });
    esp_now_register_recv_cb([](u8 *, u8 *data, u8 len) {
        Serial.print((char *) data);
        switch (device_type) {
            case 0:
                ws2812b::parse_json(reinterpret_cast<char *>(data));
                break;
            default: break;
        }
    });
}

auto msg = "Hello, esp8266!";

// unsigned long lastTime = 0;
JsonDocument doc;

void loop() {
    // if (millis() - lastTime > 2000) {
    //     // esp_now_send(broadcastAddress, (u8 *) msg, strlen(msg) + 1);
    //     Serial.println("Send broadcast message: " + String(msg));
    //     lastTime = millis();
    // }
    switch (device_type) {
        case 0:
            ws2812b::loop();
            break;
        default: break;
    }
}
