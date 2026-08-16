#include <Arduino.h>
#include "MacropadApp.h"

void setup() {
    // Forward hardware initialization and peripheral startup loops
    MacropadApp::begin();
}

void loop() {
    // Continuously cycle the polling loops, event handlers, and transport stacks
    MacropadApp::run();
}
