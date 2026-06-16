#pragma once
#include <Wire.h>
#include <esp32-hal-log.h>

// IP5306 I2C address
#define IP5306_ADDR 0x75

// Registers
#define IP5306_REG_SYS_CTL0  0x00
#define IP5306_REG_READ0     0x70
#define IP5306_REG_READ1     0x71
#define IP5306_REG_READ2     0x72
#define IP5306_REG_READ3     0x77
#define IP5306_REG_READ4     0x78

// Returns battery level as percentage (0, 25, 50, 75, 100)
// Returns -1 if IP5306 not responding (e.g. USB power without battery)
inline int battery_percent() {
    esp_log_level_set("*", ESP_LOG_NONE);  // suppress I2C errors
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_READ4);
    if (Wire.endTransmission(false) != 0) {
        esp_log_level_set("*", ESP_LOG_WARN);
        return -1;
    }

    Wire.requestFrom(IP5306_ADDR, 1);
    esp_log_level_set("*", ESP_LOG_WARN);  // restore log level
    if (!Wire.available()) return -1;

    uint8_t val = Wire.read();

    // Bits 4-1 indicate charge level
    uint8_t level = (val >> 4) & 0x0F;
    if      (level >= 12) return 100;
    else if (level >=  8) return  75;
    else if (level >=  4) return  50;
    else if (level >=  1) return  25;
    else                  return   0;
}

// Returns true if device is charging via USB
inline bool battery_charging() {
    esp_log_level_set("*", ESP_LOG_NONE);
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_READ0);
    if (Wire.endTransmission(false) != 0) {
        esp_log_level_set("*", ESP_LOG_WARN);
        return false;
    }
    Wire.requestFrom(IP5306_ADDR, 1);
    esp_log_level_set("*", ESP_LOG_WARN);
    if (!Wire.available()) return false;
    return (Wire.read() & 0x08) != 0;
}