#include "ld2420.h"
#include <ESP8266WiFi.h>
#include <espnow.h>

void ld2420::setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    ld2420.begin(115200);
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

void ld2420::loop() {
    if (ld2420.available())
        if (const String response = ld2420.readStringUntil('\n'); response.startsWith("Range")) {
            doc["data"]["status"] = 1;
            doc["data"]["range"] = response.substring(6).toInt();
            serializeJson(doc, json);
            Serial.println(json);
            json += "\n";
            esp_now_send(BROADCAST_ADDRESS, const_cast<u8 *>(reinterpret_cast<const u8 *>(json.c_str())),
                         static_cast<int>(json.length()));
        } else if (response.startsWith("OFF")) {
            doc["data"]["status"] = 0;
            doc["data"]["range"] = 0;
            serializeJson(doc, json);
            Serial.println(json);
            json += "\n";
            esp_now_send(BROADCAST_ADDRESS, const_cast<u8 *>(reinterpret_cast<const u8 *>(json.c_str())),
                         static_cast<int>(json.length()));
        }
}
