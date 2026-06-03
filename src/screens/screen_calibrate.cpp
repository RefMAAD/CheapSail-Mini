#include <lvgl.h>
#include "../wifi_comms.h"

// Original color theme
#define COL_BG     lv_color_hex(0x1a1a2e)
#define COL_CARD   lv_color_hex(0x16213e)
#define COL_ACCENT lv_color_hex(0xe94560)
#define COL_TEXT   lv_color_hex(0xf0f0f0)
#define COL_MUTED  lv_color_hex(0x888888)

static const float GAUGE_STEPS[] = { 0.05f, 0.10f, 0.20f, 0.30f, 0.50f, 1.00f };
static int         gauge_idx     = 4;
static lv_obj_t* lbl_gauge;

static void update_gauge_label() {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.2f mm", GAUGE_STEPS[gauge_idx]);
    lv_label_set_text(lbl_gauge, buf);
}

static void gauge_dec_cb(lv_event_t*) { if (gauge_idx > 0) gauge_idx--; update_gauge_label(); }
static void gauge_inc_cb(lv_event_t*) { if (gauge_idx < 5) gauge_idx++; update_gauge_label(); }
static void feeler_zero_cb(lv_event_t*) {
    char buf[64];
    snprintf(buf, sizeof(buf), "FEELER_ZERO GAUGE=%.2f", GAUGE_STEPS[gauge_idx]);
    send_gcode(buf);
}
static void accept_save_cb(lv_event_t*) {
    send_gcode("ACCEPT");
    send_gcode("SAVE_CONFIG");
}

static lv_obj_t* make_calib_btn(lv_obj_t* parent, const char* icon,
                                 const char* label, int y, lv_event_cb_t cb) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 230, 36);
    lv_obj_set_pos(btn, 5, y);
    lv_obj_set_style_bg_color(btn, COL_CARD, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x333355), 0); // Restored original
    lv_obj_set_style_border_width(btn, 1, 0);
    // Long press required for safety
    lv_obj_add_event_cb(btn, cb, LV_EVENT_LONG_PRESSED, nullptr);

    lv_obj_t* li = lv_label_create(btn);
    lv_label_set_text(li, icon);
    lv_obj_set_style_text_color(li, COL_ACCENT, 0);
    lv_obj_set_style_text_font(li, &lv_font_montserrat_14, 0);
    lv_obj_align(li, LV_ALIGN_LEFT_MID, 4, 0);

    lv_obj_t* lt = lv_label_create(btn);
    lv_label_set_text(lt, label);
    lv_obj_set_style_text_color(lt, COL_TEXT, 0);
    lv_obj_set_style_text_font(lt, &lv_font_montserrat_12, 0);
    lv_obj_align(lt, LV_ALIGN_LEFT_MID, 26, 0);
    return btn;
}

void screen_calibrate_create(lv_obj_t* parent) {
    lv_obj_set_style_bg_color(parent, COL_BG, 0);

    make_calib_btn(parent, LV_SYMBOL_SETTINGS, "Full calibration", 4,
        [](lv_event_t*){ send_gcode("FULL_CALIBRATION"); });
    make_calib_btn(parent, LV_SYMBOL_REFRESH, "Screw tilt", 46,
        [](lv_event_t*){ send_gcode("SCREWS_TILT_CALCULATE"); });
    make_calib_btn(parent, LV_SYMBOL_GPS, "Bed mesh", 88,
        [](lv_event_t*){ send_gcode("BED_MESH_CALIBRATE"); });
    make_calib_btn(parent, LV_SYMBOL_DOWN, "Z-offset", 130,
        [](lv_event_t*){ send_gcode("PROBE_CALIBRATE"); });

    // Separator
    lv_obj_t* sep = lv_obj_create(parent);
    lv_obj_set_size(sep, 230, 1);
    lv_obj_set_pos(sep, 5, 174);
    lv_obj_set_style_bg_color(sep, lv_color_hex(0x333355), 0); // Restored original
    lv_obj_set_style_border_width(sep, 0, 0);

    lv_obj_t* lbl_fw = lv_label_create(parent);
    lv_label_set_text(lbl_fw, "Feeler gauge");
    lv_obj_set_style_text_color(lbl_fw, COL_MUTED, 0);
    lv_obj_set_style_text_font(lbl_fw, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lbl_fw, 8, 180);

    lv_obj_t* btn_dec = lv_btn_create(parent);
    lv_obj_set_size(btn_dec, 36, 32);
    lv_obj_set_pos(btn_dec, 5, 198);
    lv_obj_set_style_bg_color(btn_dec, COL_CARD, 0);
    lv_obj_set_style_border_width(btn_dec, 0, 0);
    lv_obj_add_event_cb(btn_dec, gauge_dec_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t* lm = lv_label_create(btn_dec);
    lv_label_set_text(lm, "-");
    lv_obj_set_style_text_color(lm, COL_TEXT, 0);
    lv_obj_center(lm);

    lbl_gauge = lv_label_create(parent);
    lv_obj_set_style_text_color(lbl_gauge, COL_TEXT, 0);
    lv_obj_set_style_text_font(lbl_gauge, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(lbl_gauge, 52, 204);
    update_gauge_label();

    lv_obj_t* btn_inc = lv_btn_create(parent);
    lv_obj_set_size(btn_inc, 36, 32);
    lv_obj_set_pos(btn_inc, 165, 198);
    lv_obj_set_style_bg_color(btn_inc, COL_CARD, 0);
    lv_obj_set_style_border_width(btn_inc, 0, 0);
    lv_obj_add_event_cb(btn_inc, gauge_inc_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t* lp = lv_label_create(btn_inc);
    lv_label_set_text(lp, "+");
    lv_obj_set_style_text_color(lp, COL_TEXT, 0);
    lv_obj_center(lp);

    lv_obj_t* btn_fz = lv_btn_create(parent);
    lv_obj_set_size(btn_fz, 110, 34);
    lv_obj_set_pos(btn_fz, 5, 238);
    lv_obj_set_style_bg_color(btn_fz, COL_CARD, 0);
    lv_obj_set_style_border_width(btn_fz, 0, 0);
    // Long press required for safety
    lv_obj_add_event_cb(btn_fz, feeler_zero_cb, LV_EVENT_LONG_PRESSED, nullptr);
    lv_obj_t* lfz = lv_label_create(btn_fz);
    lv_label_set_text(lfz, LV_SYMBOL_OK " Calc");
    lv_obj_set_style_text_color(lfz, COL_TEXT, 0);
    lv_obj_set_style_text_font(lfz, &lv_font_montserrat_12, 0);
    lv_obj_center(lfz);

    lv_obj_t* btn_as = lv_btn_create(parent);
    lv_obj_set_size(btn_as, 110, 34);
    lv_obj_set_pos(btn_as, 123, 238);
    lv_obj_set_style_bg_color(btn_as, COL_ACCENT, 0);
    // Long press required for safety
    lv_obj_add_event_cb(btn_as, accept_save_cb, LV_EVENT_LONG_PRESSED, nullptr);
    lv_obj_t* las = lv_label_create(btn_as);
    lv_label_set_text(las, LV_SYMBOL_SAVE " Save");
    lv_obj_set_style_text_color(las, COL_TEXT, 0);
    lv_obj_set_style_text_font(las, &lv_font_montserrat_12, 0);
    lv_obj_center(las);
}