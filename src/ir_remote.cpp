#include "ir_remote.h"
#include <ESP8266WiFi.h>
#include <espnow.h>

void ir_remote::setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    analogWriteFreq(38000);
    analogWrite(LED_PIN, 0);

    if (esp_now_init()) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);

    esp_now_register_recv_cb([](u8 *, u8 *data, u8) {
        parse_json(String(reinterpret_cast<char *>(data)));
    });
}

void ir_remote::loop() {
    while (Serial.available()) {
        const String json = Serial.readStringUntil('\n');
        parse_json(json);
    }
}

inline void set_low() { analogWrite(ir_remote::LED_PIN, 240); }
inline void set_high() { analogWrite(ir_remote::LED_PIN, 0); }

inline void sent_by_time(const u32 low, const u32 high) {
    set_low();
    delayMicroseconds(low);
    set_high();
    delayMicroseconds(high);
}

inline void sent_low() { sent_by_time(640, 550); }
inline void sent_high() { sent_by_time(640, 1680); }
inline void sent_lead() { sent_by_time(9000, 4500); }
inline void sent_connection() { sent_by_time(640, 20000); }
inline void sent_end() { sent_by_time(540, 20000); }

void ir_remote::parse_json(String json) {
    if (const DeserializationError error = deserializeJson(doc, json)) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
    } else if (doc["device_name"].as<String>() == DEVICE_NAME) {
        Serial.println(json);
        u8 data1[35]{}, data2[32]{};
        u8 temp = doc["data"]["mode"].as<u8>(), check = 12;
        data1[0] = temp & 1;
        data1[1] = temp >> 1 & 1;
        data1[2] = temp >> 2 & 1;
        if (temp == 1)
            data1[23] = doc["data"]["dry_heat"].as<u8>();
        else if (temp == 4)
            data1[23] = 1 - doc["data"]["dry_heat"].as<u8>();
        check += temp;
        data1[3] = doc["data"]["power"].as<u8>();
        check += data1[3] * 8;
        temp = doc["data"]["speed"].as<u8>();
        data1[4] = temp & 1;
        data1[5] = temp >> 1 & 1;
        temp = doc["data"]["sweep"].as<u8>();
        if (temp) {
            data1[6] = 1;
            data2[0] = temp & 1;
            data2[4] = temp >> 1 & 1;
            check += data2[4];
        }
        data1[7] = doc["data"]["sleep"].as<u8>();
        temp = doc["data"]["temp"].as<u8>();
        data1[8] = temp & 1;
        data1[9] = temp >> 1 & 1;
        data1[10] = temp >> 2 & 1;
        data1[11] = temp >> 3 & 1;
        check += temp;
        temp = doc["data"]["timer"].as<u8>();
        if (temp) {
            data1[12] = temp & 1;
            data1[15] = 1;
            temp >>= 1;
            if (temp >= 20)
                data1[14] = 1;
            else if (temp >= 10)
                data1[13] = 1;
            temp %= 10;
            data1[16] = temp & 1;
            data1[17] = temp >> 1 & 1;
            data1[18] = temp >> 2 & 1;
            data1[19] = temp >> 3 & 1;
            check += temp;
        }
        data1[20] = doc["data"]["turbo"].as<u8>();
        data1[21] = doc["data"]["light"].as<u8>();
        data1[22] = doc["data"]["ion"].as<u8>();
        data1[24] = doc["data"]["venti"].as<u8>();
        check += data1[24];
        data1[28] = 1;
        data1[30] = 1;
        data1[33] = 1;
        temp = doc["data"]["display"].as<u8>();
        data2[8] = temp & 1;
        data2[9] = temp >> 1 & 1;
        data2[13] = 1;
        data2[26] = doc["data"]["economy"].as<u8>();
        check += data1[26];
        data2[28] = check & 1;
        data2[29] = check >> 1 & 1;
        data2[30] = check >> 2 & 1;
        data2[31] = check >> 3 & 1;
        sent(data1, data2);
    }
}

void ir_remote::sent(const u8 *data1, const u8 *data2) {
    sent_lead();
    for (u8 i = 0; i < 35; i++)
        data1[i] ? sent_high() : sent_low();
    sent_connection();
    for (u8 i = 0; i < 32; i++)
        data2[i] ? sent_high() : sent_low();
    sent_end();
}
