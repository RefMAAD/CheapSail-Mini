#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

// ─── Printer state structure ──────────────────────────────────────────────────
struct PrinterState {
    float   hotend_temp    = 0.0f;
    float   hotend_target  = 0.0f;
    float   bed_temp       = 0.0f;
    float   bed_target     = 0.0f;
    float   progress       = 0.0f;   // 0.0–1.0
    float   z_offset       = 0.0f;
    int32_t print_duration = 0;      // seconds
    char    state[32]      = "disconnected";
    char    filename[128]  = "";
    bool    connected      = false;
};

// ─── Public API ───────────────────────────────────────────────────────────────

// Initialise WiFi and WebSocket. Call from setup().
void wifi_init();

// Keep connection alive and process messages. Call from loop().
void wifi_loop();

// Send a G-code command to Moonraker (fire-and-forget).
void send_gcode(const char* script);

// Latest known printer state (updated automatically via WebSocket).
extern PrinterState printer;

// Device IP address — set after WiFi connects, used for OTA reference.
extern String device_ip;