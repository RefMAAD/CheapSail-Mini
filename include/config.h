#pragma once

/**
 * config.h — Project-wide settings
 * Edit this file rather than the source code directly.
 * Note: WiFi credentials set here are overridden by platformio.ini build_flags.
 */

// ─── WiFi ─────────────────────────────────────────────────────────────────────
// Can also be set in platformio.ini build_flags (-D WIFI_SSID=...)
// platformio.ini takes precedence if set in both places.
#ifndef WIFI_SSID
#define WIFI_SSID "YourNetworkName"
#endif
#ifndef WIFI_PASS
#define WIFI_PASS "YourPassword"
#endif

// ─── Moonraker ────────────────────────────────────────────────────────────────
#ifndef MOONRAKER_HOST
#define MOONRAKER_HOST "192.168.x.x"   // Raspberry Pi IP address
#endif
#ifndef MOONRAKER_PORT
#define MOONRAKER_PORT 7125
#endif

// ─── Display ─────────────────────────────────────────────────────────────────
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320
#define SCREEN_ROT    0      // 0=portrait, 1=landscape, 2=portrait flip, 3=landscape flip

// ─── Touch (CST820) ───────────────────────────────────────────────────────────
#define TOUCH_I2C_ADDR 0x15
#define TOUCH_SDA_PIN  21
#define TOUCH_SCL_PIN  22

// ─── Update rate ──────────────────────────────────────────────────────────────
#define UI_UPDATE_MS   500   // 2Hz — sufficient for temperatures, low WiFi load

// ─── Temperature limits ───────────────────────────────────────────────────────
#define EXTRUDE_MIN_TEMP 170.0f   // °C — extruder movement blocked below this
#define HOTEND_MAX_TEMP  280.0f
#define BED_MAX_TEMP     120.0f

// ─── Version ──────────────────────────────────────────────────────────────────
#define FW_VERSION "Beta 1.0r"
