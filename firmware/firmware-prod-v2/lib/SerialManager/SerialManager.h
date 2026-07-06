#pragma once

#include <Arduino.h>
#include <string>

class SerialManager {
public:
    static void begin();
    static void check();
    
    static void pushBleData(const std::string& data);

private:
    static void handleConfigRead();
    static void handleConfigWrite();
    static void processByte(char c);
    
    // Unifies reading/writing for both USB CDC and BLE NUS
    static void sendResponse(const char* str);
    static int availableBytes();
    static char readByte();
    
    static char serialBuffer[64];
    static uint8_t bufferIndex;
    static std::string bleRxQueue;
};
