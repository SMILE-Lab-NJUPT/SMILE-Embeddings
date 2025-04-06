#include "fm225.h"
#include <ESP8266WiFi.h>
#include <espnow.h>

void fm225::setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    fm225.begin(115200);
    fm225.setTimeout(100);
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

void fm225::loop() {
    fm225.write(unlock_cmd, sizeof(unlock_cmd));
    u8 buffer[64]{}, bytes_read;
    const u32 start_time = millis();
    do {
        if (millis() - start_time > 5500)
            return;
        do {
            if (millis() - start_time > 5500)
                return;
            bytes_read = fm225.readBytes(buffer, sizeof(buffer));
        } while (bytes_read == 0);
        buffer[bytes_read - 1] = 0;
    } while (buffer[0] != 0xEF || buffer[1] != 0xAA || buffer[2] != 0x00 || buffer[5] != 0x12 || buffer[6] != 0x00);
    doc["data"]["uid"] = buffer[7] << 8 | buffer[8];
    doc["data"]["username"] = String(reinterpret_cast<char *>(buffer + 9));
    Serial.println(String(reinterpret_cast<char *>(buffer + 7)));
    serializeJson(doc, json);
    Serial.println(json);
    json += "\n";
    esp_now_send(BROADCAST_ADDRESS, const_cast<u8 *>(reinterpret_cast<const u8 *>(json.c_str())),
                 static_cast<int>(json.length()));
}
