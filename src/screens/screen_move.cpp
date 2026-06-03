#include <lvgl.h>
#include "../wifi_comms.h"

// Original color theme
#define COL_BG     lv_color_hex(0x1a1a2e)
#define COL_CARD   lv_color_hex(0x16213e)
#define COL_ACCENT lv_color_hex(0xe94560)
#define COL_TEXT   lv_color_hex(0xf0f0f0)
#define COL_MUTED  lv_color_hex(0x888888)

static float step_mm = 1.0f;
static lv_obj_t* step_btns[4];
static const float STEPS[]    = { 0.1f, 1.0f, 10.0f, 50.0f };
static const char* STEP_LBLS[] = { "0.1", "1", "10", "50" };

static void highlight_step(int idx) {
    for (int i = 0; i < 4; i++)
        lv_obj_set_style_bg_color(step_btns[i], i == idx ? COL_ACCENT : COL_CARD, 0);
}

static void step_cb(lv_event_t* e) {
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    step_mm = STEPS[idx];
    highlight_step(idx);
}

typedef struct { char axis; int dir; } MoveData;
static MoveData move_data[8];

static void move_cb(lv_event_t* e) {
    MoveData* d = (MoveData*)lv_event_get_user_data(e);
    char buf[64];
    if (d->axis == 'E') {
        if (printer.hotend_temp < 170.0f) return;
        snprintf(buf, sizeof(buf), "G91\nG1 E%.2f F300\nG90", step_mm * d->dir);
    } else {
        snprintf(buf, sizeof(buf), "G91\nG0 %c%.2f F3000\nG90", d->axis, step_mm * d->dir);
    }
    send_gcode(buf);
}

static void home_cb(lv_event_t* e) {
    send_gcode((const char*)lv_event_get_user_data(e));
}

static void make_axis_row(lv_obj_t* parent, int y, char axis, int mi, lv_color_t lbl_color) {
    lv_obj_t* lbl = lv_label_create(parent);
    char al[2] = { axis, 0 };
    lv_label_set_text(lbl, al);
    lv_obj_set_style_text_color(lbl, lbl_color, 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(lbl, 0, y + 6);

    move_data[mi] = { axis, -1 };
    lv_obj_t* btn_m = lv_btn_create(parent);
    lv_obj_set_size(btn_m, 96, 32);
    lv_obj_set_pos(btn_m, 18, y);
    lv_obj_set_style_bg_color(btn_m, COL_CARD, 0);
    if (axis == 'E') { lv_obj_set_style_border_width(btn_m, 0, 0); }
    // Long press required for safety
    lv_obj_add_event_cb(btn_m, move_cb, LV_EVENT_LONG_PRESSED, &move_data[mi]);
    lv_obj_t* lm = lv_label_create(btn_m);
    lv_label_set_text(lm, axis == 'Z' ? LV_SYMBOL_DOWN " -"
                      : axis == 'E' ? "Extrude " LV_SYMBOL_RIGHT
                      : LV_SYMBOL_LEFT " -");
    lv_obj_set_style_text_color(lm, COL_TEXT, 0);
    lv_obj_center(lm);

    move_data[mi+1] = { axis, +1 };
    lv_obj_t* btn_p = lv_btn_create(parent);
    lv_obj_set_size(btn_p, 96, 32);
    lv_obj_set_pos(btn_p, 122, y);
    lv_obj_set_style_bg_color(btn_p, COL_CARD, 0);
    if (axis == 'E') { lv_obj_set_style_border_width(btn_p, 0, 0); }
    // Long press required for safety
    lv_obj_add_event_cb(btn_p, move_cb, LV_EVENT_LONG_PRESSED, &move_data[mi+1]);
    lv_obj_t* lp = lv_label_create(btn_p);
    lv_label_set_text(lp, axis == 'Z' ? "+ " LV_SYMBOL_UP
                      : axis == 'E' ? LV_SYMBOL_LEFT " Retract"
                      : "+ " LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(lp, COL_TEXT, 0);
    lv_obj_center(lp);
}

void screen_move_create(lv_obj_t* parent) {
    lv_obj_set_style_bg_color(parent, COL_BG, 0);

    lv_obj_t* lbl_mm = lv_label_create(parent);
    lv_label_set_text(lbl_mm, "mm:");
    lv_obj_set_style_text_color(lbl_mm, COL_MUTED, 0);
    lv_obj_set_style_text_font(lbl_mm, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lbl_mm, 2, 4);

    for (int i = 0; i < 4; i++) {
        step_btns[i] = lv_btn_create(parent);
        lv_obj_set_size(step_btns[i], 46, 30);
        lv_obj_set_pos(step_btns[i], 28 + i * 50, 2);
        lv_obj_set_style_bg_color(step_btns[i], i == 1 ? COL_ACCENT : COL_CARD, 0);
        lv_obj_add_event_cb(step_btns[i], step_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);
        lv_obj_t* l = lv_label_create(step_btns[i]);
        lv_label_set_text(l, STEP_LBLS[i]);
        lv_obj_set_style_text_color(l, COL_TEXT, 0);
        lv_obj_set_style_text_font(l, &lv_font_montserrat_12, 0);
        lv_obj_center(l);
    }

    make_axis_row(parent,  38, 'X', 0, COL_TEXT);
    make_axis_row(parent,  74, 'Y', 2, COL_TEXT);
    make_axis_row(parent, 110, 'Z', 4, COL_TEXT);
    make_axis_row(parent, 146, 'E', 6, COL_ACCENT);

    static const char* HOME_CMDS[] = { "G28 X", "G28 Y", "G28 Z", "G28" };
    static const char* HOME_LBLS[] = { LV_SYMBOL_HOME "X", LV_SYMBOL_HOME "Y",
                                       LV_SYMBOL_HOME "Z", LV_SYMBOL_HOME "All" };
    for (int i = 0; i < 4; i++) {
        lv_obj_t* btn = lv_btn_create(parent);
        lv_obj_set_size(btn, 52, 32);
        lv_obj_set_pos(btn, 4 + i * 56, 186);
        lv_obj_set_style_bg_color(btn, COL_CARD, 0);
        // Home protected with long press
        lv_obj_add_event_cb(btn, home_cb, LV_EVENT_LONG_PRESSED, (void*)HOME_CMDS[i]);
        lv_obj_t* l = lv_label_create(btn);
        lv_label_set_text(l, HOME_LBLS[i]);
        lv_obj_set_style_text_color(l, COL_TEXT, 0);
        lv_obj_set_style_text_font(l, &lv_font_montserrat_12, 0);
        lv_obj_center(l);
    }
}