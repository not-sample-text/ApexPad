#pragma once

#include <Arduino.h>

class MatrixScanner {
public:
    static void begin();
    static void scan();

private:
    static constexpr uint8_t MATRIX_ROWS = 4;
    static constexpr uint8_t MATRIX_COLS = 3;
    static constexpr uint8_t DEBOUNCE_DELAY_MS = 5;

    static bool stableState[MATRIX_ROWS][MATRIX_COLS];
    static bool rawStateTracking[MATRIX_ROWS][MATRIX_COLS];
    static uint32_t lastChangeTime[MATRIX_ROWS][MATRIX_COLS];
};
