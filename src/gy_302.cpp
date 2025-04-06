#include "gy_302.h"
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

void gy_302::setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    Wire.begin();
    bh1750.begin();
    doc["device_name"] = DEVICE_NAME;

    if (esp_now_init()) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_register_send_cb([](u8 *, const u8 sendStatus) {
        Serial.println("Send status: " + String(sendStatus));
    });
    esp_now_register_recv_cb([](u8 *, u8 *data, u8) {
    });
}

void gy_302::loop() {
    delay(200);
    const float lux = bh1750.readLightLevel();
    if (isnan(lux))
        return;
    doc["data"]["light"] = lux;
    serializeJson(doc, json);
    Serial.println(json);
    json += "\n";
    esp_now_send(BROADCAST_ADDRESS, const_cast<u8 *>(reinterpret_cast<const u8 *>(json.c_str())),
                 static_cast<int>(json.length()));
}
