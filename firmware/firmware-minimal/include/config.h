#pragma once

#include <cstdint>

// I2C Peripherals
constexpr uint8_t kOledAddress = 0x3C;
constexpr uint8_t kOledWidth = 128; 
constexpr uint8_t kOledHeight = 32;
constexpr uint8_t kOledSdaPin = 8;
constexpr uint8_t kOledSclPin = 9;
constexpr uint8_t kFuelGaugeAddress = 0x36;
constexpr uint8_t kFuelGaugeVcellRegister = 0x02;
constexpr uint8_t kFuelGaugeSocRegister = 0x04;

// Dedicated Digital Inputs
constexpr uint8_t kBtSelectPin = 34; // LOW = USB, HIGH = BLE
constexpr uint8_t kEncoderSwPin = 1;
constexpr uint8_t kEncoderClkPin = 2;
constexpr uint8_t kEncoderDtPin = 4;
constexpr uint8_t kVbusSensePin = 21; // 1 = Plugged, 0 = Battery

// Power & Radio Control Rails
constexpr uint8_t kStatusLedPin = 18;
constexpr uint8_t kLdo2EnablePin = 17;
constexpr uint8_t kAntennaPin = 11;    // HIGH = External Antenna
constexpr uint8_t kLedPwrEnPin = 16;   // TPS22918 Switch Enable (Keep LOW)

// Matrix Setup
constexpr uint8_t kMatrixRowCount = 4;
constexpr uint8_t kMatrixColumnCount = 3;
constexpr uint8_t kMatrixColPins[kMatrixColumnCount] = {15, 37, 35};
constexpr uint8_t kMatrixRowPins[kMatrixRowCount] = {12, 13, 14, 5};
constexpr uint32_t kMatrixDebounceMs = 5;

// Behavior Parameters
constexpr uint32_t kInitDelayMs = 1000;
constexpr uint32_t kOledInitDelayMs = 100;
constexpr uint32_t kStatusLedBlinkMs = 250;
constexpr uint32_t kEncoderHoldDurationMs = 4000;
constexpr uint32_t kBleJustConnectedDurationMs = 7500;

constexpr char kMatrixKeyMap[kMatrixRowCount][kMatrixColumnCount] = {
  {'N', '#', '*'},
  {'7', '8', '9'},
  {'4', '5', '6'},
  {'1', '2', '3'},
};

constexpr bool kEncoderDebugLogging = true;
