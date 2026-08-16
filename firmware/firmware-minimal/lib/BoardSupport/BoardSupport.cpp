#include "BoardSupport.h"
#include "config.h"

void BoardSupport::begin() {
    pinMode(kVbusSensePin, INPUT_PULLDOWN);
}

bool BoardSupport::isUsbConnected() {
    // Pin 21 is pulled down; reads HIGH when 5V USB is active
    return digitalRead(kVbusSensePin) == HIGH;
}
