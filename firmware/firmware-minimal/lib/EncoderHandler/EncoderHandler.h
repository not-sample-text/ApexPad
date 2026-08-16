#pragma once
#include <Arduino.h>

class EncoderHandler {
public:
    static void begin();
    static void run();
    static bool isSwitchPressed() { return digitalRead(1) == LOW; }
    static uint8_t getLastStateCode() { return encoderLastState; }
    static int8_t getAccumulatedSteps() { return encoderAccumulatedSteps; }
    static bool isInterruptPending();

private:
    static uint8_t readRawState();
    static uint8_t encoderLastState;
    static int8_t encoderAccumulatedSteps;
    static bool lastButtonState;
    static uint32_t lastButtonDebounce;
    static constexpr uint8_t BUTTON_DEBOUNCE_MS = 10;
};
