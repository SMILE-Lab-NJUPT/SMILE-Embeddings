#pragma once
#include <WString.h>
#include <ArduinoJson.h>
#include <BH1750.h>

namespace gy_302 {
    inline u8 BROADCAST_ADDRESS[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    inline JsonDocument doc;
    inline String json;
    inline BH1750 bh1750;

    void setup();

    void loop();
}
