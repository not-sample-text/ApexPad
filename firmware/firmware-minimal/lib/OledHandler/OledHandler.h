#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

class OledHandler {
public:
    static void begin();
    static void update();
    
    // Hardware animation transitions
    static void showBootAnimation();
    static void showSleepAnimation();
    static void clear();
    
    // Full screen blocking system message (used during Config Sync)
    static void showSystemMessage(const char* msg);

private:
    static Adafruit_SSD1306 display;
};
