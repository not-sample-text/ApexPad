#pragma once

#include <Arduino.h>
#include <string>

class BleHid {
public:
    static void begin();
    static void sendKey(uint16_t keycode, uint8_t modifiers, bool isPressed);
    static void sendConsumerKey(uint16_t consumerUsageId, bool isPressed);
    static void updateBatteryLevel(uint8_t percentage);
    static bool isBondedAndConnected();
    
    // Callbacks for the OLED UI
    static void setPasskeyShowCallback(void (*callback)(uint32_t));
    static void setPasskeyClearCallback(void (*callback)());

    // BLE Serial (NUS) Methods
    static void sendSerialData(const char* data);
    static void setSerialRxCallback(void (*callback)(const std::string& data));
};
