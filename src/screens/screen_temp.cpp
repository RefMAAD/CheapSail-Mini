#include <lvgl.h>
#include "../wifi_comms.h"

extern PrinterState printer;

#define COL_BG     lv_color_hex(0x1a1a2e)
#define COL_CARD   lv_color_hex(0x16213e)
#define COL_ACCENT lv_color_hex(0xe94560)
#define COL_HOT    lv_color_hex(0xff6b35)
#define COL_BED    lv_color_hex(0x4ecdc4)
#define COL_TEXT   lv_color_hex(0xf0f0f0)
#define COL_MUTED  lv_color_hex(0x888888)

static lv_obj_t* lbl_hot_cur;
static lv_obj_t* ta_hot;
static lv_obj_t* lbl_bed_cur;
static lv_obj_t* ta_bed;

static void set_hot_cb(lv_event_t*) {
    char buf[64];
    snprintf(buf, sizeof(buf), "SET_HEATER_TEMPERATURE HEATER=extruder TARGET=%s",
             lv_textarea_get_text(ta_hot));
    send_gcode(buf);
}
static void set_bed_cb(lv_event_t*) {
    char buf[64];
    snprintf(buf, sizeof(buf), "SET_HEATER_TEMPERATURE HEATER=heater_bed TARGET=%s",
             lv_textarea_get_text(ta_bed));
    send_gcode(buf);
}

typedef struct { const char* heater; float temp; lv_obj_t* ta; } PresetData;
static PresetData presets[6]; // Preset data structure

static void preset_cb(lv_event_t* e) {
    PresetData* pd = (PresetData*)lv_event_get_user_data(e);
    char buf[32];
    snprintf(buf, sizeof(buf), "%.0f", pd->temp);
    lv_textarea_set_text(pd->ta, buf);

    snprintf(buf, sizeof(buf), "SET_HEATER_TEMPERATURE HEATER=%s TARGET=%.0f", pd->heater, pd->temp);
    send_gcode(buf);
}

static lv_obj_t* make_btn(lv_obj_t* parent, const char* text, int x, int y, int w, int h) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_style_bg_color(btn, COL_CARD, 0);
    lv_obj_set_style_border_width(btn, 0, 0);

    lv_obj_t* l = lv_label_create(btn);
    lv_label_set_text(l, text);
    lv_obj_set_style_text_color(l, COL_TEXT, 0);
    lv_obj_center(l);
    return btn;
}

static void make_heater_block(lv_obj_t* parent, int y, const char* heater_id, lv_color_t color,
                              const char* title, lv_obj_t** cur_lbl_out, lv_obj_t** ta_out,
                              lv_event_cb_t set_cb, float t1, const char* n1, float t2, const char* n2, int pi) {
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_size(card, 230, 94);
    lv_obj_set_pos(card, 5, y);
    lv_obj_set_style_bg_color(card, COL_CARD, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_radius(card, 6, 0);
    lv_obj_set_style_pad_all(card, 0, 0);

    lv_obj_t* lt = lv_label_create(card);
    lv_label_set_text(lt, title);
    lv_obj_set_style_text_color(lt, color, 0);
    lv_obj_set_style_text_font(lt, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lt, 8, 4);

    *cur_lbl_out = lv_label_create(card);
    lv_label_set_text(*cur_lbl_out, "0.0° / 0°");
    lv_obj_set_style_text_color(*cur_lbl_out, COL_TEXT, 0);
    // Large font for temperature display
    lv_obj_set_style_text_font(*cur_lbl_out, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(*cur_lbl_out, 8, 20);

    *ta_out = lv_textarea_create(card);
    lv_obj_set_size(*ta_out, 65, 32);
    lv_obj_set_pos(*ta_out, 8, 54);
    lv_textarea_set_one_line(*ta_out, true);
    lv_textarea_set_text(*ta_out, "0");
    lv_obj_set_style_bg_color(*ta_out, COL_BG, 0);
    lv_obj_set_style_text_color(*ta_out, COL_TEXT, 0);
    lv_obj_set_style_border_width(*ta_out, 0, 0);

    lv_obj_t* btn_set = lv_btn_create(card);
    lv_obj_set_size(btn_set, 45, 32);
    lv_obj_set_pos(btn_set, 78, 54);
    lv_obj_set_style_bg_color(btn_set, COL_ACCENT, 0);
    // Set requires long press for safety
    lv_obj_add_event_cb(btn_set, set_cb, LV_EVENT_LONG_PRESSED, nullptr);
    lv_obj_t* ls = lv_label_create(btn_set);
    lv_label_set_text(ls, "Set");
    lv_obj_center(ls);

    // Preset buttons
    presets[pi] = { heater_id, t1, *ta_out };
    lv_obj_t* b1 = make_btn(card, n1, 130, 16, 92, 30);
    lv_obj_add_event_cb(b1, preset_cb, LV_EVENT_LONG_PRESSED, &presets[pi]);

    presets[pi+1] = { heater_id, t2, *ta_out };
    lv_obj_t* b2 = make_btn(card, n2, 130, 52, 92, 30);
    lv_obj_add_event_cb(b2, preset_cb, LV_EVENT_LONG_PRESSED, &presets[pi+1]);

    presets[pi+2] = { heater_id, 0.0f, *ta_out };
    lv_obj_t* b0 = make_btn(card, "Off", 82, 16, 42, 30);
    lv_obj_add_event_cb(b0, preset_cb, LV_EVENT_LONG_PRESSED, &presets[pi+2]);
}

void screen_temp_create(lv_obj_t* parent) {
    lv_obj_set_style_bg_color(parent, COL_BG, 0);

    make_heater_block(parent, 2, "extruder", COL_HOT,
        LV_SYMBOL_WARNING " Hotend",
        &lbl_hot_cur, &ta_hot, set_hot_cb,
        220, "PLA 220", 240, "ABS 240", 0);

    make_heater_block(parent, 100, "heater_bed", COL_BED,
        LV_SYMBOL_HOME " Bed",
        &lbl_bed_cur, &ta_bed, set_bed_cb,
        60, "PLA 60", 100, "ABS 100", 3);

    lv_obj_t* lbl_fw = lv_label_create(parent);
    lv_label_set_text(lbl_fw, "Filament");
    lv_obj_set_style_text_color(lbl_fw, COL_MUTED, 0);
    lv_obj_set_style_text_font(lbl_fw, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lbl_fw, 8, 200);

    lv_obj_t* btn_load = make_btn(parent, LV_SYMBOL_UP " Load",     5, 218, 110, 34);
    lv_obj_t* btn_unload = make_btn(parent, LV_SYMBOL_DOWN " Unload", 125, 218, 110, 34);

    // Filament buttons also protected with long press
    lv_obj_add_event_cb(btn_load, [](lv_event_t*){ send_gcode("M83\nG1 E50 F300"); }, LV_EVENT_LONG_PRESSED, nullptr);
    lv_obj_add_event_cb(btn_unload, [](lv_event_t*){ send_gcode("M83\nG1 E-50 F300"); }, LV_EVENT_LONG_PRESSED, nullptr);
}

void screen_temp_update() {
    char buf[48];
    snprintf(buf, sizeof(buf), "%.1f° / %.0f°", printer.hotend_temp, printer.hotend_target);
    lv_label_set_text(lbl_hot_cur, buf);

    snprintf(buf, sizeof(buf), "%.1f° / %.0f°", printer.bed_temp, printer.bed_target);
    lv_label_set_text(lbl_bed_cur, buf);
}