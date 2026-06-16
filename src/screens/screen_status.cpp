#include <lvgl.h>
#include "../wifi_comms.h"
#include "../battery.h"
#include "../../include/config.h"

extern PrinterState printer;

static lv_obj_t* lbl_state;
static lv_obj_t* lbl_hotend;
static lv_obj_t* bar_hotend;
static lv_obj_t* lbl_bed;
static lv_obj_t* bar_bed;
static lv_obj_t* lbl_filename;
static lv_obj_t* bar_progress;
static lv_obj_t* lbl_progress_pct;
static lv_obj_t* lbl_duration;

// ─── Print control buttons ─────────────────────────────────────────────────────
static lv_obj_t* btn_pause;
static lv_obj_t* btn_resume;
static lv_obj_t* btn_cancel;
static lv_obj_t* lbl_pause_resume;  // label text switches between pause/resume
static lv_obj_t* btn_home;          // home all — hidden during printing
static lv_obj_t* lbl_battery;

#define COL_BG      lv_color_hex(0x1a1a2e)
#define COL_CARD    lv_color_hex(0x16213e)
#define COL_ACCENT  lv_color_hex(0xe94560)
#define COL_HOT     lv_color_hex(0xff6b35)
#define COL_BED     lv_color_hex(0x4ecdc4)
#define COL_TEXT    lv_color_hex(0xf0f0f0)
#define COL_MUTED   lv_color_hex(0x888888)
#define COL_GREEN   lv_color_hex(0x2ecc71)
#define COL_ORANGE  lv_color_hex(0xf39c12)
#define COL_RED     lv_color_hex(0xe74c3c)

static lv_obj_t* make_bar(lv_obj_t* parent, lv_color_t color, int y) {
    lv_obj_t* bar = lv_bar_create(parent);
    lv_obj_set_size(bar, 210, 8);
    lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, y);
    lv_bar_set_range(bar, 0, 100);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x0f0f1e), 0);
    lv_obj_set_style_bg_color(bar, color, LV_PART_INDICATOR);
    return bar;
}

static void make_row(lv_obj_t* parent, const char* name, lv_obj_t** lbl_val, int y) {
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_size(card, 230, 32);
    lv_obj_set_pos(card, 5, y);
    lv_obj_set_style_bg_color(card, COL_CARD, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_radius(card, 4, 0);
    lv_obj_set_style_pad_all(card, 0, 0);

    lv_obj_t* ln = lv_label_create(card);
    lv_label_set_text(ln, name);
    lv_obj_set_style_text_color(ln, COL_MUTED, 0);
    lv_obj_set_style_text_font(ln, &lv_font_montserrat_12, 0);
    lv_obj_align(ln, LV_ALIGN_LEFT_MID, 8, 0);

    *lbl_val = lv_label_create(card);
    lv_label_set_text(*lbl_val, "-");
    lv_obj_set_style_text_color(*lbl_val, COL_TEXT, 0);
    lv_obj_set_style_text_font(*lbl_val, &lv_font_montserrat_12, 0);
    lv_obj_align(*lbl_val, LV_ALIGN_RIGHT_MID, -8, 0);
}

void screen_status_create(lv_obj_t* parent) {
    lv_obj_set_style_bg_color(parent, COL_BG, 0);

    // ── Title bar ───────────────────────────────────────────────────────────────────
    lv_obj_t* c_top = lv_obj_create(parent);
    lv_obj_set_size(c_top, 230, 40);
    lv_obj_set_pos(c_top, 5, 2);
    lv_obj_set_style_bg_color(c_top, COL_CARD, 0);
    lv_obj_set_style_border_width(c_top, 0, 0);
    lv_obj_set_style_radius(c_top, 6, 0);
    lv_obj_set_style_pad_all(c_top, 0, 0);

    lv_obj_t* l_title = lv_label_create(c_top);
    lv_label_set_text(l_title, "KLIPPER");
    lv_obj_set_style_text_color(l_title, COL_TEXT, 0);
    lv_obj_set_style_text_font(l_title, &lv_font_montserrat_14, 0);
    lv_obj_align(l_title, LV_ALIGN_LEFT_MID, 8, 0);

    lbl_state = lv_label_create(c_top);
    lv_label_set_text(lbl_state, "offline");
    lv_obj_set_style_text_color(lbl_state, COL_ACCENT, 0);
    lv_obj_set_style_text_font(lbl_state, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_state, LV_ALIGN_RIGHT_MID, -8, 0);

    // Battery indicator left of state label
    lbl_battery = lv_label_create(c_top);
    lv_label_set_text(lbl_battery, LV_SYMBOL_BATTERY_FULL);
    lv_obj_set_style_text_color(lbl_battery, COL_GREEN, 0);
    lv_obj_set_style_text_font(lbl_battery, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_battery, LV_ALIGN_LEFT_MID, 72, 0);

    // IP address shown below title bar for OTA reference
    lv_obj_t* lbl_ip = lv_label_create(parent);
    lv_obj_set_style_text_color(lbl_ip, lv_color_hex(0x444466), 0);
    lv_obj_set_style_text_font(lbl_ip, &lv_font_montserrat_10, 0);
    lv_obj_set_pos(lbl_ip, 5, 44);
    lv_label_set_text(lbl_ip, device_ip.c_str());

    // ── Temperature card ───────────────────────────────────────────────────────────
    lv_obj_t* c_temp = lv_obj_create(parent);
    lv_obj_set_size(c_temp, 230, 80);
    lv_obj_set_pos(c_temp, 5, 46);
    lv_obj_set_style_bg_color(c_temp, COL_CARD, 0);
    lv_obj_set_style_border_width(c_temp, 0, 0);
    lv_obj_set_style_radius(c_temp, 6, 0);
    lv_obj_set_style_pad_all(c_temp, 0, 0);

    lv_obj_t* lh = lv_label_create(c_temp);
    lv_label_set_text(lh, LV_SYMBOL_WARNING " Hotend");
    lv_obj_set_style_text_color(lh, COL_MUTED, 0);
    lv_obj_set_style_text_font(lh, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lh, 8, 6);

    lbl_hotend = lv_label_create(c_temp);
    lv_label_set_text(lbl_hotend, "0.0° / 0°");
    lv_obj_set_style_text_color(lbl_hotend, COL_TEXT, 0);
    lv_obj_set_style_text_font(lbl_hotend, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_hotend, LV_ALIGN_TOP_RIGHT, -8, 6);

    bar_hotend = make_bar(c_temp, COL_HOT, 24);

    lv_obj_t* lb = lv_label_create(c_temp);
    lv_label_set_text(lb, LV_SYMBOL_HOME " Bed");
    lv_obj_set_style_text_color(lb, COL_MUTED, 0);
    lv_obj_set_style_text_font(lb, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lb, 8, 42);

    lbl_bed = lv_label_create(c_temp);
    lv_label_set_text(lbl_bed, "0.0° / 0°");
    lv_obj_set_style_text_color(lbl_bed, COL_TEXT, 0);
    lv_obj_set_style_text_font(lbl_bed, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_bed, LV_ALIGN_TOP_RIGHT, -8, 42);

    bar_bed = make_bar(c_temp, COL_BED, 60);

    // ── Print progress card ────────────────────────────────────────────────────────
    lv_obj_t* c_print = lv_obj_create(parent);
    lv_obj_set_size(c_print, 230, 68);
    lv_obj_set_pos(c_print, 5, 130);
    lv_obj_set_style_bg_color(c_print, COL_CARD, 0);
    lv_obj_set_style_border_width(c_print, 0, 0);
    lv_obj_set_style_radius(c_print, 6, 0);
    lv_obj_set_style_pad_all(c_print, 0, 0);

    lbl_filename = lv_label_create(c_print);
    lv_label_set_text(lbl_filename, "No file");
    lv_obj_set_style_text_color(lbl_filename, COL_TEXT, 0);
    lv_obj_set_style_text_font(lbl_filename, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lbl_filename, 8, 4);
    lv_obj_set_width(lbl_filename, 214);
    lv_label_set_long_mode(lbl_filename, LV_LABEL_LONG_DOT);

    lbl_progress_pct = lv_label_create(c_print);
    lv_label_set_text(lbl_progress_pct, "0%");
    lv_obj_set_style_text_color(lbl_progress_pct, COL_ACCENT, 0);
    lv_obj_set_style_text_font(lbl_progress_pct, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_progress_pct, LV_ALIGN_TOP_RIGHT, -8, 24);

    bar_progress = make_bar(c_print, COL_ACCENT, 42);

    make_row(parent, "Print Time", &lbl_duration, 202);

    // ── Print control buttons ────────────────────────────────────────────────
    // Pause/Resume (left button, switches)
    btn_pause = lv_btn_create(parent);
    lv_obj_set_size(btn_pause, 110, 38);
    lv_obj_set_pos(btn_pause, 5, 238);
    lv_obj_set_style_bg_color(btn_pause, COL_ORANGE, 0);
    lv_obj_add_event_cb(btn_pause, [](lv_event_t*) {
        if (strcmp(printer.state, "printing") == 0)
            send_gcode("PAUSE");
        else if (strcmp(printer.state, "paused") == 0)
            send_gcode("RESUME");
    }, LV_EVENT_LONG_PRESSED, nullptr);
    lv_obj_add_flag(btn_pause, LV_OBJ_FLAG_HIDDEN);

    lbl_pause_resume = lv_label_create(btn_pause);
    lv_label_set_text(lbl_pause_resume, LV_SYMBOL_PAUSE " Pause");
    lv_obj_set_style_text_color(lbl_pause_resume, COL_TEXT, 0);
    lv_obj_set_style_text_font(lbl_pause_resume, &lv_font_montserrat_12, 0);
    lv_obj_center(lbl_pause_resume);

    // Resume button (same position, visibility toggled)
    btn_resume = lv_btn_create(parent);
    lv_obj_set_size(btn_resume, 110, 38);
    lv_obj_set_pos(btn_resume, 5, 238);
    lv_obj_set_style_bg_color(btn_resume, COL_GREEN, 0);
    lv_obj_add_event_cb(btn_resume, [](lv_event_t*) {
        send_gcode("RESUME");
    }, LV_EVENT_LONG_PRESSED, nullptr);
    lv_obj_add_flag(btn_resume, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t* lbl_res = lv_label_create(btn_resume);
    lv_label_set_text(lbl_res, LV_SYMBOL_PLAY " Resume");
    lv_obj_set_style_text_color(lbl_res, COL_TEXT, 0);
    lv_obj_set_style_text_font(lbl_res, &lv_font_montserrat_12, 0);
    lv_obj_center(lbl_res);

    // Cancel (right button)
    btn_cancel = lv_btn_create(parent);
    lv_obj_set_size(btn_cancel, 110, 38);
    lv_obj_set_pos(btn_cancel, 123, 238);
    lv_obj_set_style_bg_color(btn_cancel, COL_RED, 0);
    lv_obj_add_event_cb(btn_cancel, [](lv_event_t*) {
        send_gcode("CANCEL_PRINT");
    }, LV_EVENT_LONG_PRESSED, nullptr);
    lv_obj_add_flag(btn_cancel, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t* lbl_can = lv_label_create(btn_cancel);
    lv_label_set_text(lbl_can, LV_SYMBOL_CLOSE " Cancel");
    lv_obj_set_style_text_color(lbl_can, COL_TEXT, 0);
    lv_obj_set_style_text_font(lbl_can, &lv_font_montserrat_12, 0);
    lv_obj_center(lbl_can);

    // ── Home All (visible only when not printing) ──────────────────────────────────
    btn_home = lv_btn_create(parent);
    lv_obj_set_size(btn_home, 230, 38);
    lv_obj_set_pos(btn_home, 5, 238);
    lv_obj_set_style_bg_color(btn_home, COL_CARD, 0);
    lv_obj_set_style_border_width(btn_home, 0, 0);
    lv_obj_add_event_cb(btn_home, [](lv_event_t*) {
        send_gcode("G28");
    }, LV_EVENT_LONG_PRESSED, nullptr);

    lv_obj_t* lbl_home = lv_label_create(btn_home);
    lv_label_set_text(lbl_home, LV_SYMBOL_HOME "  Home All");
    lv_obj_set_style_text_color(lbl_home, COL_TEXT, 0);
    lv_obj_center(lbl_home);
}

void screen_status_update() {
    lv_label_set_text(lbl_state, printer.connected ? printer.state : "offline");

    char buf[64];
    snprintf(buf, sizeof(buf), "%.1f° / %.0f°", printer.hotend_temp, printer.hotend_target);
    lv_label_set_text(lbl_hotend, buf);
    int hot_pct = (printer.hotend_target > 0)
        ? (int)(printer.hotend_temp / printer.hotend_target * 100.0f) : 0;
    lv_bar_set_value(bar_hotend, hot_pct, LV_ANIM_ON);

    snprintf(buf, sizeof(buf), "%.1f° / %.0f°", printer.bed_temp, printer.bed_target);
    lv_label_set_text(lbl_bed, buf);
    int bed_pct = (printer.bed_target > 0)
        ? (int)(printer.bed_temp / printer.bed_target * 100.0f) : 0;
    lv_bar_set_value(bar_bed, bed_pct, LV_ANIM_ON);

    bool printing = strcmp(printer.state, "printing") == 0;
    bool paused   = strcmp(printer.state, "paused")   == 0;
    bool active   = printing || paused;

    if (active) {
        lv_label_set_text(lbl_filename, printer.filename[0] ? printer.filename : "Unknown");
        snprintf(buf, sizeof(buf), "%d%%", (int)(printer.progress * 100.0f));
        lv_label_set_text(lbl_progress_pct, buf);
        lv_bar_set_value(bar_progress, (int)(printer.progress * 100.0f), LV_ANIM_ON);

        int h = printer.print_duration / 3600;
        int m = (printer.print_duration % 3600) / 60;
        snprintf(buf, sizeof(buf), "%dh %dm", h, m);
        lv_label_set_text(lbl_duration, buf);
    } else {
        lv_label_set_text(lbl_filename, "Ready");
        lv_label_set_text(lbl_progress_pct, "0%");
        lv_bar_set_value(bar_progress, 0, LV_ANIM_ON);
        lv_label_set_text(lbl_duration, "-");
    }

    // ── Battery level ───────────────────────────────────────────────────────────────
    int bat = battery_percent();
    bool charging = battery_charging();
    if (bat < 0) {
        // USB power, no battery or IP5306 not responding
        lv_label_set_text(lbl_battery, LV_SYMBOL_USB);
        lv_obj_set_style_text_color(lbl_battery, COL_MUTED, 0);
    } else if (charging) {
        lv_label_set_text(lbl_battery, LV_SYMBOL_CHARGE);
        lv_obj_set_style_text_color(lbl_battery, COL_GREEN, 0);
    } else if (bat >= 75) {
        lv_label_set_text(lbl_battery, LV_SYMBOL_BATTERY_FULL);
        lv_obj_set_style_text_color(lbl_battery, COL_GREEN, 0);
    } else if (bat >= 50) {
        lv_label_set_text(lbl_battery, LV_SYMBOL_BATTERY_3);
        lv_obj_set_style_text_color(lbl_battery, COL_GREEN, 0);
    } else if (bat >= 25) {
        lv_label_set_text(lbl_battery, LV_SYMBOL_BATTERY_2);
        lv_obj_set_style_text_color(lbl_battery, COL_ORANGE, 0);
    } else {
        lv_label_set_text(lbl_battery, LV_SYMBOL_BATTERY_1);
        lv_obj_set_style_text_color(lbl_battery, COL_RED, 0);
    }

    // ── Show/hide buttons based on printer state ───────────────────────────────────
    if (printing) {
        lv_obj_clear_flag(btn_pause,  LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btn_resume,   LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(btn_cancel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btn_home,     LV_OBJ_FLAG_HIDDEN);
    } else if (paused) {
        lv_obj_add_flag(btn_pause,    LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(btn_resume, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(btn_cancel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btn_home,     LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(btn_pause,    LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btn_resume,   LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btn_cancel,   LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(btn_home,   LV_OBJ_FLAG_HIDDEN);
    }
}