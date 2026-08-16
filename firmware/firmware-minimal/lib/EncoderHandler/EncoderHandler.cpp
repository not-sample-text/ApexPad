#include "EncoderHandler.h"
#include "EventQueue.h"
#include "OledHandler.h"

static constexpr uint8_t kEncoderClkPin = 2;
static constexpr uint8_t kEncoderDtPin = 4;
static constexpr uint8_t kEncoderSwPin = 1;

volatile bool g_encoder_interrupt_pending = false;

void IRAM_ATTR inputManagerEncoderISR() {
    g_encoder_interrupt_pending = true;
}

uint8_t EncoderHandler::encoderLastState = 0;
int8_t EncoderHandler::encoderAccumulatedSteps = 0;
bool EncoderHandler::lastButtonState = true;
uint32_t EncoderHandler::lastButtonDebounce = 0;

uint8_t EncoderHandler::readRawState() {
    uint8_t clk = digitalRead(kEncoderClkPin) ? 1 : 0;
    uint8_t dt = digitalRead(kEncoderDtPin) ? 1 : 0;
    return static_cast<uint8_t>((clk << 1) | dt);
}

void EncoderHandler::begin() {
    pinMode(kEncoderClkPin, INPUT_PULLUP);
    pinMode(kEncoderDtPin, INPUT_PULLUP);
    pinMode(kEncoderSwPin, INPUT_PULLUP);

    encoderLastState = readRawState();

    attachInterrupt(digitalPinToInterrupt(kEncoderClkPin), inputManagerEncoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(kEncoderDtPin), inputManagerEncoderISR, CHANGE);
}

void EncoderHandler::run() {
    uint32_t now = millis();

    if (g_encoder_interrupt_pending) {
        static constexpr int8_t kTransitionTable[16] = {
            0, 1, -1, 0,
            -1, 0, 0, 1,
            1, 0, 0, -1,
            0, -1, 1, 0,
        };

        uint8_t currentState = readRawState();
        int8_t transition = kTransitionTable[(encoderLastState << 2) | currentState];
        encoderLastState = currentState;

        if (transition != 0) {
            encoderAccumulatedSteps += transition;
            if (encoderAccumulatedSteps >= 2) {
                encoderAccumulatedSteps -= 2;
                EventQueue::enqueue({EventType::EncoderCCW, 0, 0, now});
            } else if (encoderAccumulatedSteps <= -2) {
                encoderAccumulatedSteps += 2;
                EventQueue::enqueue({EventType::EncoderCW, 0, 0, now});
            }
        }
        g_encoder_interrupt_pending = false;
    }

    bool currentButton = digitalRead(kEncoderSwPin);
    if (currentButton != lastButtonState) {
        if ((now - lastButtonDebounce) > BUTTON_DEBOUNCE_MS) {
            lastButtonState = currentButton;
            lastButtonDebounce = now;
            OledHandler::update();

            if (currentButton == HIGH) {
                EventQueue::enqueue({EventType::EncoderButton, 0, 0, now});
            }
        }
    } else {
        lastButtonDebounce = now;
    }
}

bool EncoderHandler::isInterruptPending() {
    return g_encoder_interrupt_pending;
}
