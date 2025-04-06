#pragma once
#include <WString.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

namespace ld2420 {
    inline u8 BROADCAST_ADDRESS[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    inline JsonDocument doc;
    inline String json;
    inline SoftwareSerial ld2420(4, -1);

    void setup();

    void loop();
}
