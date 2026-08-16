#pragma once

#include <cstdint>

// OLED Display Configuration
constexpr uint8_t kOledAddress = 0x3C;
constexpr uint8_t kOledWidth = 128; 
constexpr uint8_t kOledHeight = 32;
constexpr uint8_t kOledSdaPin = 8; 
constexpr uint8_t kOledSclPin = 9; 

// Buttons & Encoder Pins
constexpr uint8_t kBtSelectPin = 34; 
constexpr uint8_t kEncoderSwPin = 1; 
constexpr uint8_t kEncoderClkPin = 2; 
constexpr uint8_t kEncoderDtPin = 4; 

// Power & Status Pins
constexpr uint8_t kStatusLedPin = 18; 
constexpr uint8_t kLdo2EnablePin = 17; 
constexpr uint8_t kVbusSensePin = 21; 
constexpr uint8_t kLegacyTpsEnablePin = 16;
constexpr uint8_t kLegacyRgbDataPin = 7;
constexpr uint8_t kAntennaPin = 11; 

// Fuel Gauge (MAX17048 I2C Address)
constexpr uint8_t kFuelGaugeAddress = 0x36;
constexpr uint8_t kFuelGaugeVcellRegister = 0x02;
constexpr uint8_t kFuelGaugeSocRegister = 0x04;

// Matrix Keypad Layout
constexpr uint8_t kMatrixRowCount = 4;
constexpr uint8_t kMatrixColumnCount = 3;
constexpr uint8_t kMatrixColPins[kMatrixColumnCount] = {15, 37, 35};
constexpr uint8_t kMatrixRowPins[kMatrixRowCount] = {12, 13, 14, 5};
constexpr uint32_t kMatrixDebounceMs = 5;

// Test Matrix Character Keymap
constexpr char kMatrixKeyMap[kMatrixRowCount][kMatrixColumnCount] = {
  {'N', '#', '*'},
  {'7', '8', '9'},
  {'4', '5', '6'},
  {'1', '2', '3'},
};

// Stub class to satisfy old DisplayManager logging layouts
class DebugLogger {
public:
    static const char* getMessage(uint8_t line) { return ""; }
};
