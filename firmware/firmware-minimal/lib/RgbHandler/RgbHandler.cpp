#include "RgbHandler.h"

// Explicitly define the hardware pins for a self-contained minimal build
static constexpr uint8_t kStatusLedPin = 18;
static constexpr uint8_t kLdo2EnablePin = 17;
static constexpr uint8_t kLedPwrEnPin = 16;

// Initialize the NeoPixel using your official kStatusLedPin (Pin 18)
Adafruit_NeoPixel RgbHandler::statusLed(1, kStatusLedPin, NEO_GRB + NEO_KHZ800);

void RgbHandler::begin() {
    // Keep the unused Texas Instruments TPS22918 load switch disabled to save power
    pinMode(kLedPwrEnPin, OUTPUT);
    digitalWrite(kLedPwrEnPin, LOW);
    
    // Enable LDO2 as it provides power to the NeoPixel
    pinMode(kLdo2EnablePin, OUTPUT);
    digitalWrite(kLdo2EnablePin, HIGH);
    
    statusLed.begin();
    statusLed.setBrightness(255);
    
    // Set to a solid, subtle cyan to indicate the minimal wired firmware is active
    statusLed.setPixelColor(0, statusLed.Color(0, 150, 255));
    statusLed.show();
}

void RgbHandler::run() {
    // Keeps the status LED asserted in the main loop
    statusLed.show();
}
