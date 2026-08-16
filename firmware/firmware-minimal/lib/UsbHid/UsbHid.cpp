#include "UsbHid.h"
#include <USB.h>
#include <USBHIDKeyboard.h>
#include <USBHIDConsumerControl.h>

USBHIDKeyboard Keyboard;
USBHIDConsumerControl Consumer;

void UsbHid::begin() {
    Keyboard.begin();
    Consumer.begin();
    USB.begin();
    Serial.println("Direct Raw USB HID Stack Loaded.");
}

bool UsbHid::isReady() {
    return true;
}

void UsbHid::sendKey(uint16_t keycode, uint8_t modifiers, bool isPressed) {
    if (isPressed) {
        // Apply individual raw modifiers
        if (modifiers & (1 << 0)) Keyboard.pressRaw(0xE0); // Left Ctrl
        if (modifiers & (1 << 1)) Keyboard.pressRaw(0xE1); // Left Shift
        if (modifiers & (1 << 2)) Keyboard.pressRaw(0xE2); // Left Alt
        if (modifiers & (1 << 3)) Keyboard.pressRaw(0xE3); // Left GUI

        if (keycode != 0) {
            Keyboard.pressRaw(keycode);
        }
    } else {
        if (keycode != 0) {
            Keyboard.releaseRaw(keycode);
        }
        if (modifiers & (1 << 0)) Keyboard.releaseRaw(0xE0);
        if (modifiers & (1 << 1)) Keyboard.releaseRaw(0xE1);
        if (modifiers & (1 << 2)) Keyboard.releaseRaw(0xE2);
        if (modifiers & (1 << 3)) Keyboard.releaseRaw(0xE3);

        if (keycode == 0 && modifiers == 0) {
            Keyboard.releaseAll();
        }
    }
}

// Emits consumer report ticks directly over the hardware USB endpoints
void UsbHid::sendConsumerKey(uint16_t consumerUsageId, bool isPressed) {
    if (isPressed) {
        Consumer.press(consumerUsageId);
    } else {
        Consumer.release();
    }
}
