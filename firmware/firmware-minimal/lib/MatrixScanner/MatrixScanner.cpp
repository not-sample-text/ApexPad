#include "MatrixScanner.h"
#include "EventQueue.h"

// Explicit hardware pin mappings from the true source configuration
static constexpr uint8_t ROWS_PINS[4] = {12, 13, 14, 5};
static constexpr uint8_t COLS_PINS[3] = {15, 37, 35};

bool MatrixScanner::stableState[4][3] = {false};
bool MatrixScanner::rawStateTracking[4][3] = {false};
uint32_t MatrixScanner::lastChangeTime[4][3] = {0};

void MatrixScanner::begin() {
    // Correct electrical orientation matching to-migrate source of truth:
    // Rows are initialized as high-impedance inputs
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        pinMode(ROWS_PINS[r], INPUT);
    }
    // Columns are initialized with internal pull-ups
    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        pinMode(COLS_PINS[c], INPUT_PULLUP);
    }
}

void MatrixScanner::scan() {
    uint32_t now = millis();

    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        // Drive target row LOW to act as active ground path
        pinMode(ROWS_PINS[r], OUTPUT);
        digitalWrite(ROWS_PINS[r], LOW);
        
        // Let the lines physically stabilize on the fast ESP32-S3
        delayMicroseconds(10);

        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            // Read column line (LOW = connection closed / key pressed)
            bool currentRaw = (digitalRead(COLS_PINS[c]) == LOW);

            // If state fluctuates, latch the new raw state and reset timing anchor
            if (currentRaw != rawStateTracking[r][c]) {
                rawStateTracking[r][c] = currentRaw;
                lastChangeTime[r][c] = now;
            }

            // Trigger event ONLY if the transition remains completely stable for full duration
            if ((now - lastChangeTime[r][c]) >= DEBOUNCE_DELAY_MS) {
                if (stableState[r][c] != rawStateTracking[r][c]) {
                    stableState[r][c] = rawStateTracking[r][c];

                    // Enqueue verified safe event
                    EventType type = stableState[r][c] ? EventType::KeyPress : EventType::KeyRelease;
                    EventQueue::enqueue({type, r, c, now});
                }
            }
        }

        // Revert row back to high-impedance input state
        pinMode(ROWS_PINS[r], INPUT);
    }
}
