#pragma once
#include <Arduino.h>

class MacropadApp {
public:
    static void begin();
    static void run();
    static uint8_t getCurrentLayer() { return currentLayer; }
    static const char* getLastKeyLabel() { return currentLabel; }

private:
    static void processEvents();
    static uint8_t currentLayer;
    static char currentLabel[32];
};
