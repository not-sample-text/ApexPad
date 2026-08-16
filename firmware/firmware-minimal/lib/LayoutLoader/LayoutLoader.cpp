#include "LayoutLoader.h"

JsonDocument LayoutLoader::doc;

bool LayoutLoader::begin() {
    // Mount the new LittleFS partition
    if (!LittleFS.begin(true)) {
        Serial.println("[ERROR] LittleFS mount failed!");
        return false;
    }
    
    File file = LittleFS.open("/config.json", FILE_READ);
    if (!file) {
        Serial.println("[ERROR] config.json NOT FOUND in LittleFS root!");
        return false;
    }
    
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.print("[ERROR] JSON Parse failed: ");
        Serial.println(error.c_str());
        return false;
    }

    Serial.println("[SYSTEM] config.json parsed successfully from LittleFS!");
    return true;
}

KeyAction LayoutLoader::getKeyAction(uint8_t layer, uint8_t row, uint8_t col) {
    char keyId[12];
    snprintf(keyId, sizeof(keyId), "C%uR%u", col, row);

    JsonArray layers = doc["layers"].as<JsonArray>();
    for (JsonObject layerObj : layers) {
        if (layerObj["id"] == layer) {
            JsonObject keyData = layerObj["keys"][keyId];
            
            if (!keyData.isNull()) {
                return {
                    keyData["label"].as<String>(),
                    keyData["value"].as<String>(),
                    keyData["type"].as<String>(),
                    true
                };
            }
            break;
        }
    }
    return {"UNUSED", "", "NONE", false};
}

uint8_t LayoutLoader::getPackedByte(uint8_t layer, uint8_t row, uint8_t col) {
    return (layer << 4) | ((col * 4) + row + 1);
}

static File configFile;

bool LayoutLoader::writeConfig(const char* data, bool finish) {
    if (!configFile) {
        configFile = LittleFS.open("/config.json", FILE_WRITE);
    }
    
    if (configFile) {
        configFile.print(data);
        if (finish) {
            configFile.close();
            configFile = File(); 
            Serial.println("[SYSTEM] Config write complete. Restarting...");
            delay(100);
            ESP.restart();
        }
        return true;
    }
    return false;
}
