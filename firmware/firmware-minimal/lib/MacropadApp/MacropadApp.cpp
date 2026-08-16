#include "MacropadApp.h"
#include "config.h"
#include "BoardSupport.h"
#include "MatrixScanner.h"
#include "EncoderHandler.h"
#include "OledHandler.h"
#include "RgbHandler.h"
#include "EventQueue.h"
#include "LayoutLoader.h"
#include "KeymapTranslator.h"
#include "UsbHid.h"
#include "SerialManager.h"

enum EncoderMode : uint8_t {
    MODE_VOL = 0,
    MODE_LYR = 1
};

uint8_t MacropadApp::currentLayer = 0;
char MacropadApp::currentLabel[32] = "IDLE";
EncoderMode currentEncoderMode = MODE_VOL;

void MacropadApp::begin() {
    delay(2000); // Allow hardware to stabilize
    Serial.println("[SYSTEM] Booting Minimal Wired Macropad Firmware...");
    
    BoardSupport::begin();
    LayoutLoader::begin();
    KeymapTranslator::init();
    SerialManager::begin();

    if (!LayoutLoader::begin()) {
        Serial.println("[ERROR] Failed to load config.json from flash!");
    } else {
        Serial.println("[SYSTEM] Config loaded successfully.");
    }

    MatrixScanner::begin();
    EncoderHandler::begin();
    OledHandler::begin();
    OledHandler::showBootAnimation();
    RgbHandler::begin();

    Serial.println("[SYSTEM] Initializing Native USB Transport...");
    UsbHid::begin();

    Serial.println("[SYSTEM] Application Vector Ready.");
    OledHandler::update();
}

void MacropadApp::run() {
    // Process configuration download/upload streams and remote pings
    SerialManager::check();

    // Hardware polling
    MatrixScanner::scan();
    EncoderHandler::run();
    processEvents();
    RgbHandler::run();

    delay(1); // Small delay to prevent watchdog starvation
}

void MacropadApp::processEvents() {
    InputEvent event;
    
    while (EventQueue::dequeue(event)) {
        switch (event.type) {
            case EventType::KeyPress: {
                Serial.printf("[EVENT] Key Pressed - Layer: %d, Row: %d, Col: %d\n", currentLayer, event.row, event.col);
                
                KeyAction action = LayoutLoader::getKeyAction(currentLayer, event.row, event.col);
                
                if (!action.isValid) {
                    strncpy(currentLabel, "UNUSED", sizeof(currentLabel) - 1);
                    currentLabel[sizeof(currentLabel) - 1] = '\0';
                    Serial.println("[LOG] Unmapped key pressed.");
                    OledHandler::update();
                    return;
                }

                strncpy(currentLabel, action.label.c_str(), sizeof(currentLabel) - 1);
                currentLabel[sizeof(currentLabel) - 1] = '\0';
                Serial.printf("[LOG] Action Label: %s, Type: %s\n", action.label.c_str(), action.type.c_str());

                if (action.type == "KEY" || action.type == "SHORTCUT") {
                    HidCode code;
                    if (currentLayer == 0) {
                        uint8_t linearKeyIndex = (event.row * kMatrixColumnCount) + event.col;
                        code.keycode = 104 + linearKeyIndex; 
                        code.modifiers = 0;
                    } else {
                        code = KeymapTranslator::translate(action.value);
                    }
                    UsbHid::sendKey(code.keycode, code.modifiers, true);
                }
                
                if (action.type == "APP" || action.type == "SCRIPT") {
                    uint8_t packed = LayoutLoader::getPackedByte(currentLayer, event.row, event.col);
                    Serial.printf("[CMD:%02X]\n", packed);
                }
                
                OledHandler::update();
                break;
            }
            case EventType::KeyRelease: {
                Serial.printf("[EVENT] Key Released - Layer: %d, Row: %d, Col: %d\n", currentLayer, event.row, event.col);
                
                KeyAction action = LayoutLoader::getKeyAction(currentLayer, event.row, event.col);
                
                if (action.isValid && (action.type == "KEY" || action.type == "SHORTCUT")) {
                    HidCode code;
                    if (currentLayer == 0) {
                        uint8_t linearKeyIndex = (event.row * kMatrixColumnCount) + event.col;
                        code.keycode = 104 + linearKeyIndex;
                        code.modifiers = 0;
                    } else {
                        code = KeymapTranslator::translate(action.value);
                    }
                    UsbHid::sendKey(code.keycode, code.modifiers, false);
                }
                break;
            }
            case EventType::EncoderCW:
                Serial.println("[EVENT] Encoder Turned Clockwise");
                if (currentEncoderMode == MODE_VOL) {
                    strncpy(currentLabel, "Volume Up", sizeof(currentLabel) - 1);
                    UsbHid::sendConsumerKey(0x00E9, true);
                    delay(5);
                    UsbHid::sendConsumerKey(0x00E9, false);
                } else if (currentEncoderMode == MODE_LYR) {
                    currentLayer = (currentLayer + 1) % 4;
                    strncpy(currentLabel, "Next Layer", sizeof(currentLabel) - 1);
                    Serial.printf("[LOG] Switched to Layer %d\n", currentLayer);
                }
                currentLabel[sizeof(currentLabel) - 1] = '\0';
                OledHandler::update();
                break;
                
            case EventType::EncoderCCW:
                Serial.println("[EVENT] Encoder Turned Counter-Clockwise");
                if (currentEncoderMode == MODE_VOL) {
                    strncpy(currentLabel, "Volume Down", sizeof(currentLabel) - 1);
                    UsbHid::sendConsumerKey(0x00EA, true);
                    delay(5);
                    UsbHid::sendConsumerKey(0x00EA, false);
                } else if (currentEncoderMode == MODE_LYR) {
                    currentLayer = (currentLayer == 0) ? 3 : currentLayer - 1;
                    strncpy(currentLabel, "Previous Layer", sizeof(currentLabel) - 1);
                    Serial.printf("[LOG] Switched to Layer %d\n", currentLayer);
                }
                currentLabel[sizeof(currentLabel) - 1] = '\0';
                OledHandler::update();
                break;
                
            case EventType::EncoderButton:
                Serial.println("[EVENT] Encoder Button Clicked");
                currentEncoderMode = (EncoderMode)((currentEncoderMode + 1) % 2);
                if (currentEncoderMode == MODE_VOL) {
                    strncpy(currentLabel, "Mode: Volume", sizeof(currentLabel) - 1);
                    Serial.println("[LOG] Encoder Mode Set: VOLUME");
                } else {
                    strncpy(currentLabel, "Mode: Layer", sizeof(currentLabel) - 1);
                    Serial.println("[LOG] Encoder Mode Set: LAYER");
                }
                currentLabel[sizeof(currentLabel) - 1] = '\0';
                OledHandler::update();
                break;
        }
    }
}
