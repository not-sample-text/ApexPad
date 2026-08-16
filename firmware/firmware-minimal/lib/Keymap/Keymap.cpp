#include "Keymap.h"

// Temporary definitions matching classic USB HID scan codes
#define HID_KEY_A         0x04
#define HID_KEY_C         0x06
#define HID_KEY_V         0x19
#define HID_KEY_X         0x1B
#define HID_KEY_Z         0x1D
#define HID_KEY_B         0x05
#define HID_KEY_F13       0x68

// Bitmask flags for structural shortcuts
#define MOD_LCTRL  (1 << 0)
#define MOD_LSHIFT (1 << 1)
#define MOD_LALT   (1 << 2)
#define MOD_LGUI   (1 << 3)

KeyAction Keymap::getAction(uint8_t layer, uint8_t row, uint8_t col) {
    // Fallback default safe descriptor block
    KeyAction action = {ActionType::SerialMessage, 0, 0, "NONE"};

    // Row-major index offset generation matching your physical 3x4 layout shape
    uint8_t keyIndex = (row * 3) + col;

    if (layer == 0) {
        // Layer 0: Immutable Function Keys F13 to F24
        action.type = ActionType::HidKey;
        action.keycode = HID_KEY_F13 + keyIndex;
        action.modifiers = 0;
        
        static char labelBuf[4][3][8];
        snprintf(labelBuf[row][col], 8, "F%d", 13 + keyIndex);
        action.label = labelBuf[row][col];
        return action;
    } 
    else if (layer == 1) {
        // Layer 1: High-Utility Interactive Desktop Shortcuts
        action.type = ActionType::HidShortcut;
        action.modifiers = MOD_LCTRL; // Base control assignment default

        switch (keyIndex) {
            case 0: action.keycode = HID_KEY_A; action.label = "Ctrl+A"; break;
            case 1: action.keycode = HID_KEY_C; action.label = "Ctrl+C"; break;
            case 2: action.keycode = HID_KEY_V; action.label = "Ctrl+V"; break;
            case 3: action.keycode = HID_KEY_Z; action.label = "Ctrl+Z"; break;
            case 4: action.keycode = HID_KEY_X; action.label = "Ctrl+X"; break;
            
            case 5: // Win + Ctrl + Shift + B (Graphics Driver Reset sequence)
                action.keycode = HID_KEY_B;
                action.modifiers = MOD_LGUI | MOD_LCTRL | MOD_LSHIFT;
                action.label = "W+C+S+B";
                break;
                
            case 6:  action.keycode = HID_KEY_Z; action.modifiers = MOD_LCTRL | MOD_LSHIFT; action.label = "Redo"; break;
            case 7:  action.keycode = 0x2B; action.modifiers = 0; action.label = "TAB"; break; // Tab
            case 8:  action.keycode = 0x2C; action.modifiers = 0; action.label = "SPC"; break; // Space
            case 9:  action.keycode = 0x28; action.modifiers = 0; action.label = "ENT"; break; // Enter
            case 10: action.keycode = 0x2A; action.modifiers = 0; action.label = "BSPC"; break; // Backspace
            default:
                action.type = ActionType::HidKey;
                action.keycode = HID_KEY_F13 + keyIndex;
                action.modifiers = 0;
                action.label = "F-Key";
                break;
        }
        return action;
    }
    else {
        // Layers 2 & 3 (User indices 3 & 4): Raw host communication loops
        action.type = ActionType::SerialMessage;
        action.keycode = 0;
        action.modifiers = 0;

        // Formats to 1-based indexing nomenclature cleanly
        static char msgBuf[4][3][40];
        snprintf(msgBuf[row][col], 40, "layer %d: col %d row %d pressed", layer + 1, col + 1, row + 1);
        action.label = msgBuf[row][col];
        return action;
    }
}
