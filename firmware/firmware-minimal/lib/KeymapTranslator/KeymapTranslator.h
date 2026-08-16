#pragma once

#include <Arduino.h>
#include <cstdint>

struct HidCode {
    uint8_t keycode;
    uint8_t modifiers;
};

class KeymapTranslator {
public:
    static void init();
    static HidCode translate(const String& value);
};
