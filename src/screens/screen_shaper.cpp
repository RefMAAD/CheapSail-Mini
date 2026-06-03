#include <lvgl.h>
#include "../wifi_comms.h"

// Original color theme including COL_TEAL
#define COL_BG     lv_color_hex(0x1a1a2e)
#define COL_CARD   lv_color_hex(0x16213e)
#define COL_ACCENT lv_color_hex(0xe94560)
#define COL_TEAL   lv_color_hex(0x4ecdc4)
#define COL_TEXT   lv_color_hex(0xf0f0f0)
#define COL_MUTED  lv_color_hex(0x888888)

static lv_obj_t* lbl_x_result;
static lv_obj_t* lbl_y_result;

static lv_obj_t* make_shaper_btn(lv_obj_t* parent, const char* icon,
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

void screen_shaper_create(lv_obj_t* parent) {
    lv_obj_set_style_bg_color(parent, COL_BG, 0);

    lv_obj_t* lbl_title = lv_label_create(parent);
    lv_label_set_text(lbl_title, "Input Shaper");
    lv_obj_set_style_text_color(lbl_title, COL_MUTED, 0);
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lbl_title, 8, 4);

    // X card
    lv_obj_t* card_x = lv_obj_create(parent);
    lv_obj_set_size(card_x, 110, 36);
    lv_obj_set_pos(card_x, 5, 22);
    lv_obj_set_style_bg_color(card_x, COL_CARD, 0);
    lv_obj_set_style_border_width(card_x, 0, 0);
    lv_obj_set_style_radius(card_x, 5, 0);
    lv_obj_set_style_pad_all(card_x, 4, 0);

    lv_obj_t* lx = lv_label_create(card_x);
    lv_label_set_text(lx, "X-axis");
    lv_obj_set_style_text_color(lx, COL_MUTED, 0);
    lv_obj_set_style_text_font(lx, &lv_font_montserrat_10, 0);
    lv_obj_align(lx, LV_ALIGN_TOP_LEFT, 0, 0);

    lbl_x_result = lv_label_create(card_x);
    lv_label_set_text(lbl_x_result, "MZV 57Hz");
    lv_obj_set_style_text_color(lbl_x_result, COL_TEAL, 0); // Original teal color
    lv_obj_set_style_text_font(lbl_x_result, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_x_result, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    // Y card
    lv_obj_t* card_y = lv_obj_create(parent);
    lv_obj_set_size(card_y, 110, 36);
    lv_obj_set_pos(card_y, 123, 22);
    lv_obj_set_style_bg_color(card_y, COL_CARD, 0);
    lv_obj_set_style_border_width(card_y, 0, 0);
    lv_obj_set_style_radius(card_y, 5, 0);
    lv_obj_set_style_pad_all(card_y, 4, 0);

    lv_obj_t* ly = lv_label_create(card_y);
    lv_label_set_text(ly, "Y-axis");
    lv_obj_set_style_text_color(ly, COL_MUTED, 0);
    lv_obj_set_style_text_font(ly, &lv_font_montserrat_10, 0);
    lv_obj_align(ly, LV_ALIGN_TOP_LEFT, 0, 0);

    lbl_y_result = lv_label_create(card_y);
    lv_label_set_text(lbl_y_result, "MZV 31Hz");
    lv_obj_set_style_text_color(lbl_y_result, COL_TEAL, 0); // Original teal color
    lv_obj_set_style_text_font(lbl_y_result, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_y_result, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    make_shaper_btn(parent, "~",             "Measure noise",   66,
        [](lv_event_t*){ send_gcode("MEASURE_AXES_NOISE"); });
    make_shaper_btn(parent, LV_SYMBOL_AUDIO, "Test X",          108,
        [](lv_event_t*){ send_gcode("G28\nTEST_RESONANCES AXIS=X"); });
    make_shaper_btn(parent, LV_SYMBOL_AUDIO, "Test Y",          150,
        [](lv_event_t*){ send_gcode("G28\nTEST_RESONANCES AXIS=Y"); });
    make_shaper_btn(parent, LV_SYMBOL_SETTINGS, "Auto-calibrate", 192,
        [](lv_event_t*){ send_gcode("G28\nSHAPER_CALIBRATE"); });

    lv_obj_t* btn_save = lv_btn_create(parent);
    lv_obj_set_size(btn_save, 230, 34);
    lv_obj_set_pos(btn_save, 5, 236);
    lv_obj_set_style_bg_color(btn_save, COL_ACCENT, 0);
    // Long press required for safety
    lv_obj_add_event_cb(btn_save, [](lv_event_t*){ send_gcode("SAVE_CONFIG"); },
                        LV_EVENT_LONG_PRESSED, nullptr);
    lv_obj_t* ls = lv_label_create(btn_save);
    lv_label_set_text(ls, LV_SYMBOL_SAVE " Save + Restart");
    lv_obj_set_style_text_color(ls, COL_TEXT, 0);
    lv_obj_set_style_text_font(ls, &lv_font_montserrat_12, 0);
    lv_obj_center(ls);
}

void screen_shaper_update(const char* x_str, const char* y_str) {
    lv_label_set_text(lbl_x_result, x_str);
    lv_label_set_text(lbl_y_result, y_str);
}