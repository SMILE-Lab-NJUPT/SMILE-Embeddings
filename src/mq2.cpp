#include "mq2.h"
#include <ESP8266WiFi.h>
#include <espnow.h>

void mq2::setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    pinMode(STATE_PIN, INPUT_PULLUP);
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

void mq2::loop() {
    delay(500);
    doc["data"]["smoke"] = 1 - digitalRead(STATE_PIN);
    serializeJson(doc, json);
    Serial.println(json);
    json += "\n";
    esp_now_send(BROADCAST_ADDRESS, const_cast<u8 *>(reinterpret_cast<const u8 *>(json.c_str())),
                 static_cast<int>(json.length()));
}
