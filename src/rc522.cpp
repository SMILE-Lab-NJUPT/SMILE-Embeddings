#include "rc522.h"
#include <ESP8266WiFi.h>
#include <espnow.h>

void rc522::setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    SPI.begin();
    rfid.PCD_Init();
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

void rc522::loop() {
    if (!rfid.PICC_IsNewCardPresent())
        return;

    if (!rfid.PICC_ReadCardSerial())
        return;

    std::vector<u8> uid;

    for (u8 i = 0; i < rfid.uid.size; i++)
        uid.push_back(rfid.uid.uidByte[i]);

    Serial.print("UID: ");
    for (u32 i = 0; i < uid.size(); i++) {
        if (uid[i] < 0x10)
            Serial.print("0");
        Serial.print(uid[i], HEX);
        if (i != uid.size() - 1)
            Serial.print(":");
    }
    Serial.println();

    if (uid == ACCEPTED_UID) {
        doc["data"]["state"] = 1;
        serializeJson(doc, json);
        Serial.println(json);
        json += "\n";
        esp_now_send(BROADCAST_ADDRESS, const_cast<u8 *>(reinterpret_cast<const u8 *>(json.c_str())),
                     static_cast<int>(json.length()));
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}
