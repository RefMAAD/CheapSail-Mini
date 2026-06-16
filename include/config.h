#pragma once

/**
 * config.h — Project-wide settings
 * Edit this file rather than the source code directly.
 * Note: WiFi credentials set here are overridden by platformio.ini build_flags.
 */

// ─── WiFi ─────────────────────────────────────────────────────────────────────
#ifndef WIFI_SSID
#define WIFI_SSID "YourNetworkName"
#endif
#ifndef WIFI_PASS
#define WIFI_PASS "YourPassword"
#endif

// ─── Moonraker ────────────────────────────────────────────────────────────────
#ifndef MOONRAKER_HOST
#define MOONRAKER_HOST "192.168.x.x"
#endif
#ifndef MOONRAKER_PORT
#define MOONRAKER_PORT 7125
#endif

// ─── Display ─────────────────────────────────────────────────────────────────
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320
#define SCREEN_ROT    0

// ─── Touch (CST820) ───────────────────────────────────────────────────────────
#define TOUCH_I2C_ADDR 0x15
#define TOUCH_SDA_PIN  21
#define TOUCH_SCL_PIN  22

// ─── Update rate ──────────────────────────────────────────────────────────────
#define UI_UPDATE_MS   500

// ─── Temperature limits ───────────────────────────────────────────────────────
#define EXTRUDE_MIN_TEMP 170.0f
#define HOTEND_MAX_TEMP  280.0f
#define BED_MAX_TEMP     120.0f

// ─── Version ──────────────────────────────────────────────────────────────────
#define FW_VERSION "Beta 1.0r OTA"