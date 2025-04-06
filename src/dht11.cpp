#include "dht11.h"
#include <ESP8266WiFi.h>
#include <espnow.h>

void dht11::setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    dht.setup(DATA_PIN);
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

void dht11::loop() {
    delay(5000);
    const float temp = dht.getTemperature(), hum = dht.getHumidity();
    if (isnan(temp) || isnan(hum))
        return;
    doc["data"]["temp"] = temp;
    doc["data"]["humidity"] = hum;
    serializeJson(doc, json);
    Serial.println(json);
    json += "\n";
    esp_now_send(BROADCAST_ADDRESS, const_cast<u8 *>(reinterpret_cast<const u8 *>(json.c_str())),
                 static_cast<int>(json.length()));
}
