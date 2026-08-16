#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "config.h"

class BatteryManager {
public:
    static bool getPercent(uint8_t& percent) {
        uint16_t socRaw = 0;
        
        if (!read16(kFuelGaugeSocRegister, socRaw)) return false;
        
        percent = (socRaw >> 8) & 0xFF;
        
        if (percent > 100) percent = 100;
        
        return true;
    }

    static bool getVoltageMv(uint16_t& voltageMv) {
        uint16_t vcellRaw = 0;
        if (!read16(kFuelGaugeVcellRegister, vcellRaw)) return false;
        uint64_t millivolts = (static_cast<uint64_t>(vcellRaw) * 78125ULL + 500000ULL) / 1000000ULL;
        voltageMv = static_cast<uint16_t>(millivolts);
        return true;
    }

private:
    static bool read16(uint8_t reg, uint16_t& value) {
        Wire.beginTransmission(kFuelGaugeAddress);
        Wire.write(reg);
        if (Wire.endTransmission(false) != 0) return false;
        if (Wire.requestFrom(static_cast<int>(kFuelGaugeAddress), 2) != 2) return false;
        uint8_t msb = Wire.read();
        uint8_t lsb = Wire.read();
        value = (static_cast<uint16_t>(msb) << 8) | lsb;
        return true;
    }
};
