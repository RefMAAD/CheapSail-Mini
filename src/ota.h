#pragma once
/**
 * ota.h — Over-The-Air firmware update support
 * CheapSail Mini — Beta 1.0r
 * Author: RefMAAD
 *
 * Usage:
 *   ota_init()  — call from setup() after WiFi connects
 *   ota_loop()  — call from loop()
 *
 * To flash OTA with PlatformIO, add to platformio.ini:
 *   upload_protocol = espota
 *   upload_port     = 192.168.x.x   (device IP shown on status screen)
 *   upload_flags    = --auth=cheapsail
 */

#include <ArduinoOTA.h>
#include <lvgl.h>
#include <WiFi.h>
#include "wifi_comms.h"

// ─── OTA progress UI ─────────────────────────────────────────────────────────
static lv_obj_t* ota_overlay  = nullptr;
static lv_obj_t* ota_bar      = nullptr;
static lv_obj_t* ota_label    = nullptr;

static void ota_ui_create() {
    lv_obj_t* scr = lv_scr_act();

    // Full-screen dark overlay
    ota_overlay = lv_obj_create(scr);
    lv_obj_set_size(ota_overlay, 240, 320);
    lv_obj_align(ota_overlay, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(ota_overlay, lv_color_hex(0x0a0a1a), 0);
    lv_obj_set_style_bg_opa(ota_overlay, LV_OPA_90, 0);
    lv_obj_set_style_border_width(ota_overlay, 0, 0);
    lv_obj_set_style_radius(ota_overlay, 0, 0);

    // Title
    lv_obj_t* title = lv_label_create(ota_overlay);
    lv_label_set_text(title, LV_SYMBOL_DOWNLOAD " OTA Update");
    lv_obj_set_style_text_color(title, lv_color_hex(0xe94560), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -40);

    // Progress bar
    ota_bar = lv_bar_create(ota_overlay);
    lv_obj_set_size(ota_bar, 200, 12);
    lv_obj_align(ota_bar, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_range(ota_bar, 0, 100);
    lv_bar_set_value(ota_bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(ota_bar, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_bg_color(ota_bar, lv_color_hex(0xe94560), LV_PART_INDICATOR);

    // Status label
    ota_label = lv_label_create(ota_overlay);
    lv_label_set_text(ota_label, "Starting...");
    lv_obj_set_style_text_color(ota_label, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(ota_label, &lv_font_montserrat_12, 0);
    lv_obj_align(ota_label, LV_ALIGN_CENTER, 0, 30);

    lv_timer_handler();
}

static void ota_ui_update(int pct, const char* msg) {
    if (!ota_bar || !ota_label) return;
    lv_bar_set_value(ota_bar, pct, LV_ANIM_ON);
    lv_label_set_text(ota_label, msg);
    lv_timer_handler();
}

// ─── OTA init ────────────────────────────────────────────────────────────────
void ota_init() {
    ArduinoOTA.setHostname("CheapSail-Mini");
    ArduinoOTA.setPassword("cheapsail");

    ArduinoOTA.onStart([]() {
        Serial.println("[OTA] starting");
        ota_ui_create();
        ota_ui_update(0, "Receiving...");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        int pct = progress * 100 / total;
        char buf[24];
        snprintf(buf, sizeof(buf), "%d%%", pct);
        ota_ui_update(pct, buf);
        Serial.printf("[OTA] %d%%\n", pct);
    });

    ArduinoOTA.onEnd([]() {
        ota_ui_update(100, "Done! Rebooting...");
        Serial.println("[OTA] complete");
        delay(800);
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("[OTA] error: %u\n", error);
        if (ota_label)
            lv_label_set_text(ota_label, "Error! Reboot device.");
        lv_timer_handler();
        delay(2000);
        if (ota_overlay) lv_obj_del(ota_overlay);
        ota_overlay = ota_bar = ota_label = nullptr;
    });

    ArduinoOTA.begin();
    Serial.printf("[OTA] ready, IP: %s\n", device_ip.c_str());
}

// ─── OTA loop ────────────────────────────────────────────────────────────────
void ota_loop() {
    ArduinoOTA.handle();
}