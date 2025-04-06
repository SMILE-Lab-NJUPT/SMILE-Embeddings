#pragma once
#include <WString.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

namespace fm225 {
    inline u8 BROADCAST_ADDRESS[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    inline JsonDocument doc;
    inline String json;
    inline SoftwareSerial fm225(4, 5);
    const u8 unlock_cmd[] = {0xEF, 0xAA, 0x12, 0x00, 0x02, 0x00, 0x05, 0x15};
    inline u32 last_time = 0;

    void setup();

    void loop();
}
