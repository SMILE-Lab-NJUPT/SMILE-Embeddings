#pragma once
#include <WString.h>
#include <ArduinoJson.h>
#include <MFRC522.h>

namespace rc522 {
    inline u8 BROADCAST_ADDRESS[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    constexpr u8 RST_PIN = 5;
    constexpr u8 SS_PIN = 15;
    inline JsonDocument doc;
    inline String json;
    inline MFRC522 rfid(SS_PIN, RST_PIN);
    inline std::vector<u8> ACCEPTED_UID = {0x44, 0xA2, 0xD0, 0x04};

    void setup();

    void loop();
}
