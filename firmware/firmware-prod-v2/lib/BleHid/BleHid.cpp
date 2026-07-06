#include "BleHid.h"
#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include "RgbHandler.h"

// Nordic UART Service (NUS) UUIDs
#define NUS_SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define NUS_CHARACTERISTIC_RX_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define NUS_CHARACTERISTIC_TX_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

static NimBLEHIDDevice* hidDevice = nullptr;
static NimBLECharacteristic* inputKeyboard = nullptr;
static NimBLECharacteristic* inputConsumer = nullptr;
static NimBLECharacteristic* batteryLevelChar = nullptr;

// NUS Characteristics and callback pointer
static NimBLECharacteristic* nusTxChar = nullptr;
static NimBLECharacteristic* nusRxChar = nullptr;
static void (*onNusRxData)(const std::string&) = nullptr;

static bool connectedState = false;
static bool bondedState = false;

static void (*onPasskeyDisplay)(uint32_t) = nullptr;
static void (*onPasskeyClear)() = nullptr;

static uint8_t keyboardReportBuffer[8] = {0};

static const uint8_t hidReportMap[] = {
    // Keyboard Report (ID 1)
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0xE0,        //   Usage Minimum (0xE0)
    0x29, 0xE7,        //   Usage Maximum (0xE7)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x65,        //   Logical Maximum (101)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0x00,        //   Usage Minimum (0x00)
    0x29, 0x65,        //   Usage Maximum (0x65)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection

    // Consumer Control Report (ID 2)
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x02,        //   Report ID (2)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x03,  //   Logical Maximum (1023)
    0x19, 0x00,        //   Usage Minimum (0)
    0x2A, 0xFF, 0x03,  //   Usage Maximum (1023)
    0x75, 0x10,        //   Report Size (16)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0               // End Collection
};

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) override {
        connectedState = true;
        pServer->updateConnParams(desc->conn_handle, 6, 12, 0, 400);
        RgbHandler::setBleState(BleLedState::kJustConnected);
    }

    void onDisconnect(NimBLEServer* pServer) override {
        connectedState = false;
        bondedState = false;
        RgbHandler::setBleState(BleLedState::kDisconnected);
        NimBLEDevice::startAdvertising();
    }
};

class SecurityCallbacks : public NimBLESecurityCallbacks {
    uint32_t onPassKeyRequest() override { return 0; }
    
    void onPassKeyNotify(uint32_t pass_key) override {
        if (onPasskeyDisplay) onPasskeyDisplay(pass_key);
    }
    
    bool onConfirmPIN(uint32_t pass_key) override { return true; }
    bool onSecurityRequest() override { return true; }
    
    void onAuthenticationComplete(ble_gap_conn_desc* desc) override {
        if (desc->sec_state.encrypted) {
            bondedState = true;
            if (onPasskeyClear) onPasskeyClear();
        } else {
            NimBLEDevice::getServer()->disconnect(desc->conn_handle);
        }
    }
};

// Callback for Host writing to NUS RX Characteristic
class NusCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) override {
        std::string rxValue = pCharacteristic->getValue();
        if (rxValue.length() > 0 && onNusRxData != nullptr) {
            onNusRxData(rxValue);
        }
    }
};

void BleHid::setPasskeyShowCallback(void (*callback)(uint32_t)) {
    onPasskeyDisplay = callback;
}

void BleHid::setPasskeyClearCallback(void (*callback)()) {
    onPasskeyClear = callback;
}

void BleHid::setSerialRxCallback(void (*callback)(const std::string& data)) {
    onNusRxData = callback;
}

void BleHid::sendSerialData(const char* data) {
    if (isBondedAndConnected() && nusTxChar != nullptr) {
        nusTxChar->setValue((uint8_t*)data, strlen(data));
        nusTxChar->notify();
    }
}

void BleHid::begin() {
    NimBLEDevice::init("ApexPad");
    
    NimBLEServer* server = NimBLEDevice::createServer();
    server->setCallbacks(new ServerCallbacks());

    hidDevice = new NimBLEHIDDevice(server);
    inputKeyboard = hidDevice->inputReport(1); 
    inputConsumer = hidDevice->inputReport(2); 

    hidDevice->manufacturer()->setValue("ApexPad");
    hidDevice->pnp(0x02, 0xe502, 0xa111, 0x0210);
    hidDevice->hidInfo(0x00, 0x01);
    hidDevice->reportMap((uint8_t*)hidReportMap, sizeof(hidReportMap));

    NimBLEService* batteryService = server->createService(NimBLEUUID((uint16_t)0x180F));
    batteryLevelChar = batteryService->createCharacteristic(
        NimBLEUUID((uint16_t)0x2A19),
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    batteryService->start();

    // Initialize Nordic UART Service
    NimBLEService* nusService = server->createService(NUS_SERVICE_UUID);
    
    nusTxChar = nusService->createCharacteristic(
        NUS_CHARACTERISTIC_TX_UUID,
        NIMBLE_PROPERTY::NOTIFY
    );
    
    nusRxChar = nusService->createCharacteristic(
        NUS_CHARACTERISTIC_RX_UUID,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );
    nusRxChar->setCallbacks(new NusCallbacks());
    nusService->start();

    hidDevice->startServices();

    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
    NimBLEDevice::setSecurityInitKey(BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID);
    NimBLEDevice::setSecurityRespKey(BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID);
    NimBLEDevice::setSecurityCallbacks(new SecurityCallbacks());

    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
    advertising->setAppearance(0x03C1); 
    advertising->addServiceUUID(hidDevice->hidService()->getUUID());
    advertising->addServiceUUID(nusService->getUUID());
    advertising->start();

    Serial.println("Secure BLE Stack armed. NUS Activated. NVS Bonding Active.");
}

bool BleHid::isBondedAndConnected() {
    return connectedState && bondedState;
}

void BleHid::updateBatteryLevel(uint8_t percentage) {
    if (batteryLevelChar) {
        batteryLevelChar->setValue(&percentage, 1);
        batteryLevelChar->notify();
    }
}

void BleHid::sendKey(uint16_t keycode, uint8_t modifiers, bool isPressed) {
    if (!isBondedAndConnected()) return;

    if (isPressed) {
        keyboardReportBuffer[0] |= modifiers; 
        if (keycode != 0) {
            keyboardReportBuffer[2] = (uint8_t)keycode; 
        }
    } else {
        keyboardReportBuffer[0] &= ~modifiers; 
        if (keycode != 0 && keyboardReportBuffer[2] == (uint8_t)keycode) {
            keyboardReportBuffer[2] = 0;
        }
        if (keycode == 0 && modifiers == 0) {
            memset(keyboardReportBuffer, 0, 8);
        }
    }

    inputKeyboard->setValue(keyboardReportBuffer, 8);
    inputKeyboard->notify();
}

void BleHid::sendConsumerKey(uint16_t consumerUsageId, bool isPressed) {
    if (!isBondedAndConnected()) return;

    uint8_t consumerBuffer[2] = {0};
    if (isPressed) {
        consumerBuffer[0] = (uint8_t)(consumerUsageId & 0xFF);
        consumerBuffer[1] = (uint8_t)((consumerUsageId >> 8) & 0xFF);
    }

    inputConsumer->setValue(consumerBuffer, 2);
    inputConsumer->notify();
}
