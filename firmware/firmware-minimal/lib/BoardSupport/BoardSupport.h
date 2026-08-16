#pragma once
#include <Arduino.h>

class BoardSupport {
public:
    static void begin();
    static bool isUsbConnected();
};
