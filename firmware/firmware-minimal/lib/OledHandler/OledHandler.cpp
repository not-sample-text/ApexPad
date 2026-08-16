#include "OledHandler.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MacropadApp.h"
#include "BoardSupport.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1

Adafruit_SSD1306 OledHandler::display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

extern uint8_t currentEncoderMode;

static const unsigned char PROGMEM lightning_bolt[] = {
    0x20, 0x60, 0x40, 0xC0, 0xF8, 0x10, 0x30, 0x20, 0x40
};

void OledHandler::begin() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("[ERROR] SSD1306 allocation failed"));
        return;
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();
}

const char* getLayerLongName(uint8_t layer) {
    switch (layer) {
        case 0:  return "FN KEYS";
        case 1:  return "SHORTCUTS";
        case 2:  return "COMMANDS";
        case 3:  return "APPS";
        default: return "UNKNOWN";
    }
}

const char* getEncoderModeString() {
    switch (currentEncoderMode) {
        case 0:  return "VOLUME";
        case 1:  return "LAYER SELECT";
        default: return "UNKNOWN";
    }
}

void OledHandler::update() {
    display.clearDisplay();
    display.setTextWrap(false);

    // Header: Layer Name & USB Indicator
    display.setFont(nullptr);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(getLayerLongName(MacropadApp::getCurrentLayer()));

    if (BoardSupport::isUsbConnected()) {
        display.drawBitmap(120, 0, lightning_bolt, 5, 9, SSD1306_WHITE);
    }

    // Body: Centered Active Key / Action Label
    display.setTextSize(1);
    const char* nickname = MacropadApp::getLastKeyLabel();

    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(nickname, 0, 12, &x1, &y1, &w, &h);
    int centerTextX = (128 - w) / 2;
    if (centerTextX < 0) centerTextX = 0;

    display.setCursor(centerTextX, 12);
    display.print(nickname);

    // Footer: Active Encoder Mode
    display.setTextSize(1);
    display.setCursor(0, 24);
    display.print(getEncoderModeString());

    display.display();
}

void OledHandler::showSystemMessage(const char* msg) {
    display.clearDisplay();
    display.setFont(nullptr);
    display.setTextSize(2);

    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(msg, 0, 10, &x1, &y1, &w, &h);
    display.setCursor((128 - w) / 2, 10);
    display.print(msg);

    display.display();
}

void OledHandler::clear() {
    display.clearDisplay();
    display.display();
}

void OledHandler::showBootAnimation() {
    display.clearDisplay();
    for (int16_t x = 0; x <= SCREEN_WIDTH; x += 4) {
        display.fillRect(0, 0, x, SCREEN_HEIGHT, SSD1306_WHITE);
        display.display();
    }
    delay(150);
}
