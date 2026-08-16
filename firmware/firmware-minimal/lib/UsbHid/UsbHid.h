#pragma once

#include <Arduino.h>

class UsbHid {
public:
    static void begin();
    static bool isReady();
    
    // Updated to uint16_t and added the consumer media method
    static void sendKey(uint16_t keycode, uint8_t modifiers, bool isPressed);
    static void sendConsumerKey(uint16_t consumerUsageId, bool isPressed);
};
