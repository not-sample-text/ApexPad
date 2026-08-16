#pragma once

#include <Arduino.h>

enum class ActionType : uint8_t {
    HidKey,
    HidShortcut,
    ConsumerControl, // ── ADDED: Explicit media key identifier ──
    SerialMessage
};

struct KeyAction {
    ActionType type;
    uint16_t   keycode;   // Upgraded to uint16_t to comfortably support large Consumer usage IDs
    uint8_t    modifiers; 
    const char* label;    
};

class Keymap {
public:
    static KeyAction getAction(uint8_t layer, uint8_t row, uint8_t col);
};
