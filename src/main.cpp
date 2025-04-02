#ifdef LIGHT
#include "ws2812b.h"

void setup() {
    ws2812b::setup();
}

void loop(){
    ws2812b::loop();
}
#endif

#ifdef LIGHTSENSOR
#include "gy_302.h"

void setup() {
    gy_302::setup();
}

void loop(){
    gy_302::loop();
}
#endif

#ifdef HUMANSENSOR
#include "ld2420.h"

void setup() {
    ld2420::setup();
}

void loop(){
    ld2420::loop();
}
#endif

#ifdef AIRCONDITIONER
#include "ir_remote.h"

void setup() {
    ir_remote::setup();
}

void loop() {
    ir_remote::loop();
}
#endif

#ifdef SMOKESENSOR
#include "mq2.h"

void setup() {
    mq2::setup();
}

void loop() {
    mq2::loop();
}
#endif

#ifdef TEMPHUMSENSOR
#include "dht11.h"

void setup() {
    dht11::setup();
}

void loop(){
    dht11::loop();
}
#endif

#ifdef NFC
#include "rc522.h"

void setup() {
    rc522::setup();
}

void loop() {
    rc522::loop();
}
#endif
