#pragma once
#include "Events.h"

class EventQueue {
public:
    static constexpr uint8_t CAPACITY = 32;
    static bool enqueue(InputEvent event);
    static bool dequeue(InputEvent& event);
    static bool isEmpty();
private:
    static InputEvent buffer[CAPACITY];
    static volatile uint8_t head;
    static volatile uint8_t tail;
};
