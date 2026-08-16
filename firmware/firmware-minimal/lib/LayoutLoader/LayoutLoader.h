#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

struct KeyAction {
    String label;
    String value;
    String type;
    bool isValid;
};

class LayoutLoader {
public:
    static bool begin();
    static KeyAction getKeyAction(uint8_t layer, uint8_t row, uint8_t col);
    static uint8_t getPackedByte(uint8_t layer, uint8_t row, uint8_t col);
    static JsonDocument& getDoc() { return doc; }
    
    static bool writeConfig(const char* data, bool finish);

private:
    static JsonDocument doc;
};
