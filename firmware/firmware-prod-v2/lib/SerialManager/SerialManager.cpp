#include "SerialManager.h"
#include <SPIFFS.h>
#include "OledHandler.h"
#include "BleHid.h"
#include "MacropadApp.h"

char SerialManager::serialBuffer[64];
uint8_t SerialManager::bufferIndex = 0;
std::string SerialManager::bleRxQueue = "";

void SerialManager::begin() {
    Serial.begin(115200);
    bufferIndex = 0;
    memset(serialBuffer, 0, sizeof(serialBuffer));
    bleRxQueue.clear();
    
    // Bind the BLE RX event to this manager
    BleHid::setSerialRxCallback([](const std::string& data) {
        SerialManager::pushBleData(data);
    });
}

void SerialManager::pushBleData(const std::string& data) {
    bleRxQueue += data;
}

int SerialManager::availableBytes() {
    return Serial.available() + bleRxQueue.length();
}

char SerialManager::readByte() {
    if (bleRxQueue.length() > 0) {
        char c = bleRxQueue[0];
        bleRxQueue.erase(0, 1);
        return c;
    }
    return Serial.read();
}

void SerialManager::sendResponse(const char* str) {
    if (MacropadApp::isBleMode()) {
        BleHid::sendSerialData(str);
    } else {
        Serial.print(str);
    }
}

void SerialManager::check() {
    while (availableBytes() > 0) {
        processByte(readByte());
    }
}

void SerialManager::processByte(char c) {
    if (c == '\n' || c == '\r' || c == ']') {
        if (c == ']') {
            if (bufferIndex < sizeof(serialBuffer) - 1) {
                serialBuffer[bufferIndex++] = c;
            }
        }
        
        serialBuffer[bufferIndex] = '\0';
        String cmd(serialBuffer);

        if (cmd.indexOf("[PING]") >= 0) {
            sendResponse("[PONG:APEXPAD]\n");
        } else if (cmd.indexOf("[CFG_READ_REQ]") >= 0) {
            handleConfigRead();
        } else if (cmd.indexOf("[CFG_WRITE_REQ]") >= 0) {
            handleConfigWrite();
        }

        bufferIndex = 0;
        memset(serialBuffer, 0, sizeof(serialBuffer));
    } else {
        if (bufferIndex < sizeof(serialBuffer) - 1) {
            serialBuffer[bufferIndex++] = c;
        }
    }
}

void SerialManager::handleConfigRead() {
    sendResponse("[CFG_READ_START]\n");
    
    File file = SPIFFS.open("/config.json", FILE_READ);
    if (file) {
        char buf[64]; 
        while (file.available()) {
            size_t len = file.readBytes(buf, sizeof(buf) - 1);
            buf[len] = '\0';
            sendResponse(buf);
        }
        file.close();
    }
    
    sendResponse("\n[CFG_READ_END]\n");
}

void SerialManager::handleConfigWrite() {
    OledHandler::showSystemMessage("SYNCING...");
    
    if (SPIFFS.exists("/config.json")) {
        SPIFFS.rename("/config.json", "/config.bak");
    }
    
    File file = SPIFFS.open("/config.json", FILE_WRITE);
    sendResponse("[CFG_WRITE_ACK]\n");
    
    uint32_t lastDataTime = millis();
    String currentLine = "";
    
    // Blocking loop perfectly supported via unified availableBytes() and readByte()
    while (true) {
        while (availableBytes() > 0) {
            char c = readByte();
            if (c == '\n' || c == '\r') {
                if (currentLine.indexOf("[CFG_WRITE_EOF]") >= 0) {
                    file.close();
                    SPIFFS.remove("/config.bak");
                    sendResponse("[CFG_WRITE_OK]\n");
                    OledHandler::showSystemMessage("SUCCESS");
                    delay(1000);
                    ESP.restart();
                } else {
                    if (currentLine.length() > 0) {
                        file.print(currentLine);
                        file.print("\n");
                    }
                }
                currentLine = "";
            } else {
                currentLine += c;
            }
            lastDataTime = millis();
        }
        
        // Timeout Failsafe (5 Seconds)
        if (millis() - lastDataTime > 5000) {
            file.close();
            SPIFFS.remove("/config.json");
            
            if (SPIFFS.exists("/config.bak")) {
                SPIFFS.rename("/config.bak", "/config.json");
            }
            
            sendResponse("[CFG_WRITE_ERR]\n");
            OledHandler::showSystemMessage("ERROR");
            delay(2000);
            ESP.restart();
        }
    }
}
