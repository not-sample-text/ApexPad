#include "SerialManager.h"
#include <LittleFS.h>
#include "OledHandler.h"

char SerialManager::serialBuffer[64];
uint8_t SerialManager::bufferIndex = 0;

void SerialManager::begin() {
    Serial.begin(115200);
    
    // Tell the Native USB CDC stack to instantly drop overflow logs 
    // rather than freezing the entire board waiting for the host to read them.
    Serial.setTxTimeoutMs(0);
    
    bufferIndex = 0;
    memset(serialBuffer, 0, sizeof(serialBuffer));
}

void SerialManager::sendResponse(const char* str) {
    Serial.print(str);
}

void SerialManager::check() {
    while (Serial.available() > 0) {
        processByte(static_cast<char>(Serial.read()));
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
    
    File file = LittleFS.open("/config.json", FILE_READ);
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
    
    if (LittleFS.exists("/config.json")) {
        LittleFS.rename("/config.json", "/config.bak");
    }
    
    File file = LittleFS.open("/config.json", FILE_WRITE);
    sendResponse("[CFG_WRITE_ACK]\n");
    
    uint32_t lastDataTime = millis();
    String currentLine = "";
    
    while (true) {
        while (Serial.available() > 0) {
            char c = static_cast<char>(Serial.read());
            if (c == '\n' || c == '\r') {
                if (currentLine.indexOf("[CFG_WRITE_EOF]") >= 0) {
                    file.close();
                    LittleFS.remove("/config.bak");
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
        
        // Timeout failsafe (5 seconds)
        if (millis() - lastDataTime > 5000) {
            file.close();
            LittleFS.remove("/config.json");
            
            if (LittleFS.exists("/config.bak")) {
                LittleFS.rename("/config.bak", "/config.json");
            }
            
            sendResponse("[CFG_WRITE_ERR]\n");
            OledHandler::showSystemMessage("ERROR");
            delay(2000);
            ESP.restart();
        }
    }
}
