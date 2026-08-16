#pragma once
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

class RgbHandler {
public:
    static void begin();
    static void run();

private:
    static Adafruit_NeoPixel statusLed;
};
