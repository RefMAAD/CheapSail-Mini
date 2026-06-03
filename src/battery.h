#pragma once
#include <Wire.h>

// IP5306 I2C osoite
#define IP5306_ADDR 0x75

// Rekisterit
#define IP5306_REG_SYS_CTL0  0x00
#define IP5306_REG_READ0     0x70
#define IP5306_REG_READ1     0x71
#define IP5306_REG_READ2     0x72
#define IP5306_REG_READ3     0x77
#define IP5306_REG_READ4     0x78

// Palauttaa akun varauksen prosentteina (0, 25, 50, 75, 100)
// Palauttaa -1 jos IP5306 ei vastaa (esim. USB-virta ilman akkua)
inline int battery_percent() {
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_READ4);
    if (Wire.endTransmission(false) != 0) return -1;

    Wire.requestFrom(IP5306_ADDR, 1);
    if (!Wire.available()) return -1;

    uint8_t val = Wire.read();

    // Bitit 4-1 kertovat varauksen
    uint8_t level = (val >> 4) & 0x0F;
    if      (level >= 12) return 100;
    else if (level >=  8) return  75;
    else if (level >=  4) return  50;
    else if (level >=  1) return  25;
    else                  return   0;
}

// Palauttaa true jos laite latautuu USB:stä
inline bool battery_charging() {
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_READ0);
    if (Wire.endTransmission(false) != 0) return false;
    Wire.requestFrom(IP5306_ADDR, 1);
    if (!Wire.available()) return false;
    return (Wire.read() & 0x08) != 0;
}