#pragma once

#include <Arduino.h>

enum class EventType : uint8_t {
    KeyPress,
    KeyRelease,
    EncoderCW,
    EncoderCCW,
    EncoderButton
};

struct InputEvent {
    EventType type;
    uint8_t   row;       
    uint8_t   col;       // Fixed typo here
    uint32_t  timestamp; 
};
