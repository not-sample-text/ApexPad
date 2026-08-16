#include "MacropadApp.h"
#include "config.h"
#include "BoardSupport.h"
#include "BatteryManager.h"
#include "MatrixScanner.h"
#include "EncoderHandler.h"
#include "OledHandler.h"
#include "RgbHandler.h"
#include "EventQueue.h"
#include "LayoutLoader.h"
#include "KeymapTranslator.h"
#include "UsbHid.h"
#include "BleHid.h"
#include "SerialManager.h"

enum EncoderMode : uint8_t {
    MODE_VOL = 0,
    MODE_LYR = 1
};

uint8_t MacropadApp::currentLayer = 0;
bool MacropadApp::bleModeActive = false;
bool MacropadApp::lastBleSwitchState = false;
uint32_t MacropadApp::lastActivityMs = 0;
char MacropadApp::currentLabel[32] = "IDLE";

EncoderMode currentEncoderMode = MODE_VOL;
uint32_t bleSecurePasskeyCached = 0;
bool displayPasskeyActive = false;

static uint32_t switchDebounceTimer = 0;
static bool candidateSwitchState = false;

void displayPasskeyOnOled(uint32_t passkey) {
    bleSecurePasskeyCached = passkey;
    displayPasskeyActive = true;
    MacropadApp::resetActivityTimer();
    OledHandler::update();
}

void clearPasskeyOnOled() {
    displayPasskeyActive = false;
    OledHandler::update();
}

void MacropadApp::begin() {
    delay(2000);
    
    BoardSupport::begin();
    
    LayoutLoader::begin();
    KeymapTranslator::init();
    SerialManager::begin();

    if (!LayoutLoader::begin()) {
        Serial.println("[DEBUG] Failed to load config.json from SPIFFS!");
    }

    delay(50);

    bleModeActive = BoardSupport::isBleSwitchActive();
    lastBleSwitchState = bleModeActive;
    candidateSwitchState = bleModeActive;

    MatrixScanner::begin();
    EncoderHandler::begin();
    
    OledHandler::begin();
    OledHandler::showBootAnimation();
    
    RgbHandler::begin();

    BleHid::setPasskeyShowCallback(displayPasskeyOnOled);
    BleHid::setPasskeyClearCallback(clearPasskeyOnOled);

    if (bleModeActive) {
        Serial.println("Booting strictly into BLE Transport...");
        BleHid::begin();
        RgbHandler::setBleState(BleLedState::kPairing);
    } else {
        Serial.println("Booting strictly into Native USB Transport...");
        UsbHid::begin();
    }

    lastActivityMs = millis();
    Serial.println("ApexPad Application Vector Ready.");
    
    OledHandler::update();
}

void MacropadApp::run() {
    uint32_t now = millis();
    
    // Process configuration download/upload streams and remote pings
    SerialManager::check();

    bool currentBleSwitch = BoardSupport::isBleSwitchActive();
    
    if (currentBleSwitch != candidateSwitchState) {
        candidateSwitchState = currentBleSwitch;
        switchDebounceTimer = now;
    }

    if ((currentBleSwitch != lastBleSwitchState) && ((now - switchDebounceTimer) > 500)) {
        Serial.println("Transport Layer switch state stabilized. Forcing clean state restart...");
        delay(100);
        ESP.restart();
    }

    static uint32_t lastBatteryUpdate = 0;
    static uint8_t lastBatPct = 255; // Initialize to an impossible value so it updates on first run
    
    if (now - lastBatteryUpdate >= 30000 || lastBatteryUpdate == 0) {
        uint8_t batPct = 0;
        if (BatteryManager::getPercent(batPct)) {
            if (bleModeActive) {
                BleHid::updateBatteryLevel(batPct);
            }
            
            // Check if the battery percentage has changed since the last check
            if (batPct != lastBatPct) {
                lastBatPct = batPct;
                // Only update the OLED if it's above 0% to prevent overriding the sleep animation
                if (batPct > 0) {
                    OledHandler::update();
                }
            }
            
            if (batPct == 0 && !BoardSupport::isUsbConnected()) {
                OledHandler::showSleepAnimation();
                OledHandler::clear();
                BoardSupport::enterDeepSleep(true);
            }
        }
        lastBatteryUpdate = now;
    }

    static bool lastVbusState = BoardSupport::isUsbConnected();
    bool currentVbus = BoardSupport::isUsbConnected();
    if (currentVbus != lastVbusState) {
        lastVbusState = currentVbus;
        OledHandler::update();
    }

    MatrixScanner::scan();
    EncoderHandler::run();
    processEvents();
    RgbHandler::run();

    checkInactivityTimeout(now);

    delay(1);
}

void MacropadApp::processEvents() {
    InputEvent event;
    
    while (EventQueue::dequeue(event)) {
        lastActivityMs = millis();

        switch (event.type) {
            case EventType::KeyPress: {
                KeyAction action = LayoutLoader::getKeyAction(currentLayer, event.row, event.col);
                
                if (!action.isValid) {
                    strncpy(currentLabel, "UNUSED", sizeof(currentLabel) - 1);
                    currentLabel[sizeof(currentLabel) - 1] = '\0';
                    OledHandler::update();
                    return;
                }

                strncpy(currentLabel, action.label.c_str(), sizeof(currentLabel) - 1);
                currentLabel[sizeof(currentLabel) - 1] = '\0';

                if (action.type == "KEY" || action.type == "SHORTCUT") {
                    HidCode code;
                    
                    if (currentLayer == 0) {
                        uint8_t linearKeyIndex = (event.row * kMatrixColumnCount) + event.col;
                        code.keycode = 104 + linearKeyIndex; 
                        code.modifiers = 0;
                    } else {
                        code = KeymapTranslator::translate(action.value);
                    }

                    if (bleModeActive) BleHid::sendKey(code.keycode, code.modifiers, true);
                    else               UsbHid::sendKey(code.keycode, code.modifiers, true);
                }
                
                if (action.type == "APP" || action.type == "SCRIPT") {
                    uint8_t packed = LayoutLoader::getPackedByte(currentLayer, event.row, event.col);
                    Serial.printf("[CMD:%02X]\n", packed);
                }
                
                OledHandler::update();
                break;
            }
            case EventType::KeyRelease: {
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
                    if (bleModeActive) BleHid::sendKey(code.keycode, code.modifiers, false);
                    else               UsbHid::sendKey(code.keycode, code.modifiers, false);
                }
                break;
            }
            case EventType::EncoderCW:
                if (currentEncoderMode == MODE_VOL) {
                    strncpy(currentLabel, "Volume Up", sizeof(currentLabel) - 1);
                    if (bleModeActive) BleHid::sendConsumerKey(0x00E9, true);  
                    else               UsbHid::sendConsumerKey(0x00E9, true);
                    delay(5);
                    if (bleModeActive) BleHid::sendConsumerKey(0x00E9, false); 
                    else               UsbHid::sendConsumerKey(0x00E9, false);
                } else if (currentEncoderMode == MODE_LYR) {
                    strncpy(currentLabel, "Next Layer", sizeof(currentLabel) - 1);
                    currentLayer = (currentLayer + 1) % 4;
                }
                currentLabel[sizeof(currentLabel) - 1] = '\0';
                OledHandler::update();
                break;
            case EventType::EncoderCCW:
                if (currentEncoderMode == MODE_VOL) {
                    strncpy(currentLabel, "Volume Down", sizeof(currentLabel) - 1);
                    if (bleModeActive) BleHid::sendConsumerKey(0x00EA, true);  
                    else               UsbHid::sendConsumerKey(0x00EA, true);
                    delay(5);
                    if (bleModeActive) BleHid::sendConsumerKey(0x00EA, false); 
                    else               UsbHid::sendConsumerKey(0x00EA, false);
                } else if (currentEncoderMode == MODE_LYR) {
                    strncpy(currentLabel, "Previous Layer", sizeof(currentLabel) - 1);
                    currentLayer = (currentLayer == 0) ? 3 : currentLayer - 1;
                }
                currentLabel[sizeof(currentLabel) - 1] = '\0';
                OledHandler::update();
                break;
            case EventType::EncoderButton:
                currentEncoderMode = (EncoderMode)((currentEncoderMode + 1) % 2);
                if (currentEncoderMode == MODE_VOL) strncpy(currentLabel, "Mode: Volume", sizeof(currentLabel) - 1);
                else strncpy(currentLabel, "Mode: Layer", sizeof(currentLabel) - 1);
                currentLabel[sizeof(currentLabel) - 1] = '\0';
                OledHandler::update();
                break;
        }
    }
}

void MacropadApp::checkInactivityTimeout(uint32_t now) {
    if (BoardSupport::isUsbConnected() || displayPasskeyActive) {
        lastActivityMs = now;
        return;
    }
    
    uint32_t elapsed = now - lastActivityMs;
    if (elapsed >= DEEP_SLEEP_TIMEOUT_MS) {
        OledHandler::showSleepAnimation();
        OledHandler::clear();
        
        BoardSupport::enterDeepSleep(false);
    }
}
