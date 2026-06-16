#include <Arduino.h>
#include <lvgl.h>
#include "wifi_comms.h"
#include "ota.h"

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ST7789   _panel_instance;
    lgfx::Bus_Parallel8  _bus_instance;
    lgfx::Light_PWM      _light_instance;
    lgfx::Touch_CST816S  _touch_instance;

public:
    LGFX(void) {
        {
            auto cfg = _bus_instance.config();
            cfg.freq_write = 20000000;
            cfg.pin_wr  =  4;
            cfg.pin_rd  =  2;
            cfg.pin_rs  = 16;
            cfg.pin_d0  = 15;
            cfg.pin_d1  = 13;
            cfg.pin_d2  = 12;
            cfg.pin_d3  = 14;
            cfg.pin_d4  = 27;
            cfg.pin_d5  = 25;
            cfg.pin_d6  = 33;
            cfg.pin_d7  = 32;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs   = 17;
            cfg.pin_rst  = -1;
            cfg.pin_busy = -1;
            cfg.memory_width  = 240;
            cfg.memory_height = 320;
            cfg.panel_width   = 240;
            cfg.panel_height  = 320;
            cfg.offset_x      = 0;
            cfg.offset_y      = 0;
            cfg.offset_rotation = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits  = 1;
            cfg.readable      = false;
            cfg.invert        = false;
            cfg.rgb_order     = false;
            cfg.dlen_16bit    = false;
            cfg.bus_shared    = false;
            _panel_instance.config(cfg);
        }
        {
            auto cfg = _light_instance.config();
            cfg.pin_bl      = 23;
            cfg.invert      = false;
            cfg.freq        = 44100;
            cfg.pwm_channel = 7;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }
        {
            auto cfg = _touch_instance.config();
            cfg.x_min      = 0;
            cfg.x_max      = 239;
            cfg.y_min      = 0;
            cfg.y_max      = 319;
            cfg.pin_int    = -1;
            cfg.bus_shared = true;
            cfg.offset_rotation = 0;
            cfg.i2c_port   = 0;
            cfg.i2c_addr   = 0x15;
            cfg.pin_sda    = 21;
            cfg.pin_scl    = 22;
            cfg.freq       = 400000;
            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }
        setPanel(&_panel_instance);
    }
};

static LGFX lcd;

// ─── LVGL flush ───────────────────────────────────────────────────────────────
#define DISP_BUF_LINES 10
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[240 * DISP_BUF_LINES];

static void disp_flush(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    lcd.startWrite();
    lcd.setAddrWindow(area->x1, area->y1, w, h);
    lcd.writePixels((lgfx::swap565_t*)color_p, w * h);
    lcd.endWrite();
    lv_disp_flush_ready(drv);
}

// ─── Touch + swipe ────────────────────────────────────────────────────────────
static int16_t  swipe_x0    = -1;
static uint32_t swipe_t0    = 0;
static bool     swipe_active = false;

#define SWIPE_MIN_PX  40
#define SWIPE_MAX_MS 600

static void touch_read(lv_indev_drv_t*, lv_indev_data_t* data) {
    uint16_t x, y;
    if (lcd.getTouch(&x, &y)) {
        data->point.x = x;
        data->point.y = y;
        data->state   = LV_INDEV_STATE_PR;

        if (!swipe_active) {
            swipe_x0     = x;
            swipe_t0     = millis();
            swipe_active = true;
        }
    } else {
        if (swipe_active) {
            // Touch released — swipe check handled below
        }
        swipe_active = false;
        data->state  = LV_INDEV_STATE_REL;
    }
}

static lv_obj_t* tab_pages[5];
static int        active_tab = 0;
#define TAB_COUNT 5

// ─── Tab indicator dots ────────────────────────────────────────────────────────
static lv_obj_t* tab_dots[TAB_COUNT];
#define COL_BG     lv_color_hex(0x1a1a2e)
#define COL_ACCENT lv_color_hex(0xe94560)
#define COL_MUTED  lv_color_hex(0x444466)
#define COL_TEXT   lv_color_hex(0xf0f0f0)

static void update_dots() {
    for (int i = 0; i < TAB_COUNT; i++)
        lv_obj_set_style_bg_color(tab_dots[i], i == active_tab ? COL_ACCENT : COL_MUTED, 0);
}

void switch_tab(int idx) {
    if (idx == active_tab || idx < 0 || idx >= TAB_COUNT) return;
    lv_obj_add_flag(tab_pages[active_tab], LV_OBJ_FLAG_HIDDEN);
    active_tab = idx;
    lv_obj_clear_flag(tab_pages[active_tab], LV_OBJ_FLAG_HIDDEN);
    update_dots();
}

// ─── Swipe detection in loop ───────────────────────────────────────────────────
static int16_t last_touch_x = -1;
static bool    was_pressed   = false;

static void check_swipe_loop() {
    uint16_t x, y;
    bool pressed = lcd.getTouch(&x, &y);
    if (pressed) {
        if (!was_pressed) {
            swipe_x0 = x;
            swipe_t0 = millis();
        }
        last_touch_x = x;
        was_pressed  = true;
    } else {
        if (was_pressed && swipe_x0 >= 0) {
            int32_t dx = (int32_t)last_touch_x - (int32_t)swipe_x0;
            uint32_t dt = millis() - swipe_t0;
            if (dt < SWIPE_MAX_MS) {
                if (dx < -SWIPE_MIN_PX) switch_tab(active_tab + 1); // swipe left → next
                else if (dx > SWIPE_MIN_PX) switch_tab(active_tab - 1); // swipe right → prev
            }
        }
        was_pressed  = false;
        last_touch_x = -1;
        swipe_x0     = -1;
    }
}

static void lv_tick_task(void*) { lv_tick_inc(2); }

// ─── Screen declarations ─────────────────────────────────────────────────────
extern void screen_status_create(lv_obj_t* parent);
extern void screen_status_update();
extern void screen_move_create(lv_obj_t* parent);
extern void screen_temp_create(lv_obj_t* parent);
extern void screen_temp_update();
extern void screen_calibrate_create(lv_obj_t* parent);
extern void screen_shaper_create(lv_obj_t* parent);

static const char* TAB_NAMES[] = { "Status", "Move", "Temp", "Cal.", "IS" };

static void ui_init() {
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, COL_BG, 0);

    // ── Tab pages (full screen 320px tall) ──────────────────────────────────────
    for (int i = 0; i < TAB_COUNT; i++) {
        tab_pages[i] = lv_obj_create(scr);
        lv_obj_set_size(tab_pages[i], 240, 308);
        lv_obj_align(tab_pages[i], LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_bg_color(tab_pages[i], COL_BG, 0);
        lv_obj_set_style_border_width(tab_pages[i], 0, 0);
        lv_obj_set_style_radius(tab_pages[i], 0, 0);
        lv_obj_set_style_pad_all(tab_pages[i], 0, 0);
        if (i != 0) lv_obj_add_flag(tab_pages[i], LV_OBJ_FLAG_HIDDEN);
    }

    screen_status_create(tab_pages[0]);
    screen_move_create(tab_pages[1]);
    screen_temp_create(tab_pages[2]);
    screen_calibrate_create(tab_pages[3]);
    screen_shaper_create(tab_pages[4]);

    // ── Dot indicators at bottom ────────────────────────────────────────────────
    lv_obj_t* dot_bar = lv_obj_create(scr);
    lv_obj_set_size(dot_bar, 240, 12);
    lv_obj_align(dot_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(dot_bar, lv_color_hex(0x0f0f1e), 0);
    lv_obj_set_style_border_width(dot_bar, 0, 0);
    lv_obj_set_style_radius(dot_bar, 0, 0);
    lv_obj_set_style_pad_all(dot_bar, 0, 0);
    lv_obj_set_flex_flow(dot_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(dot_bar, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_cross_place(dot_bar, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_column(dot_bar, 8, 0);

    for (int i = 0; i < TAB_COUNT; i++) {
        tab_dots[i] = lv_obj_create(dot_bar);
        lv_obj_set_size(tab_dots[i], 8, 8);
        lv_obj_set_style_radius(tab_dots[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(tab_dots[i], 0, 0);
        lv_obj_set_style_bg_color(tab_dots[i], i == 0 ? COL_ACCENT : COL_MUTED, 0);
        lv_obj_set_style_pad_all(tab_dots[i], 0, 0);
    }
}

// Convert RGB888 -> RGB565 and apply fade (0=black, 100=full brightness)
uint16_t fadeColor(uint32_t rgb888, uint8_t percentage) {
    if (percentage == 0) return 0x0000;

    uint32_t r = (rgb888 >> 16) & 0xFF;
    uint32_t g = (rgb888 >>  8) & 0xFF;
    uint32_t b =  rgb888        & 0xFF;

    if (percentage < 100) {
        r = (r * percentage) / 100;
        g = (g * percentage) / 100;
        b = (b * percentage) / 100;
    }

    // RGB888 -> RGB565
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("[boot] start");

    lcd.init();
    lcd.setRotation(0);
    lcd.setBrightness(255);

    // ── Splash screen with fade-in animation ───────────────────────────────────
    lcd.fillScreen(0x0000); // Start from fully black screen

    // Fade loop: increase brightness 0% -> 100% in 10 steps
    for (int pct = 0; pct <= 100; pct += 10) {
        
        // Calculate faded colors for this step
        uint16_t bg      = fadeColor(0x1a1a2e, pct);
        uint16_t red     = fadeColor(0xe94560, pct);
        uint16_t white   = fadeColor(0xf0f0f0, pct);
        uint16_t teal    = fadeColor(0x4ecdc4, pct);
        uint16_t text    = fadeColor(0xf0f0f0, pct);
        uint16_t muted   = fadeColor(0x444466, pct);

        // Draw background
        lcd.fillScreen(bg);

        // --- SAILBOAT SILHOUETTE ---
        // Mast
        lcd.drawFastVLine(120, 30, 120, fadeColor(0x888888, pct));
        // Main sail
        lcd.fillTriangle(120, 35, 120, 145, 165, 120, red);
        // Foresail
        lcd.fillTriangle(120, 50, 120, 140, 82, 125, white);
        // Hull
        lcd.fillTriangle(80, 150, 160, 150, 155, 165, teal);
        lcd.fillTriangle(80, 150, 155, 165, 75, 165, teal);
        
        // Waves
        lcd.drawFastHLine(60, 170, 120, teal);
        lcd.drawFastHLine(55, 174, 10, teal);
        lcd.drawFastHLine(75, 174, 15, teal);
        lcd.drawFastHLine(100, 174, 20, teal);
        lcd.drawFastHLine(130, 174, 12, teal);
        lcd.drawFastHLine(152, 174, 18, teal);

        // --- TEXT ---
        lcd.setTextSize(3);
        lcd.setCursor(39, 200);
        lcd.setTextColor(text);
        lcd.print("CheapSail");

        lcd.setTextSize(2);
        lcd.setCursor(96, 235);
        lcd.setTextColor(red);
        lcd.print("Mini");

        lcd.setTextSize(2);
        lcd.setCursor(54, 265);
        lcd.setTextColor(muted);
        lcd.print("for Klipper");

        delay(40); // 40ms * 10 steps = ~0.4s elegant fade-in
    }

    // Logo fully visible — hold for 3.5 seconds
    delay(3500);
    
    // Clear screen before LVGL init
    lcd.fillScreen(0x000000);

    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf1, nullptr, 240 * DISP_BUF_LINES);
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res  = 240;
    disp_drv.ver_res  = 320;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type    = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touch_read;
    lv_indev_drv_register(&indev_drv);

    esp_timer_handle_t timer;
    const esp_timer_create_args_t args = { .callback = lv_tick_task, .name = "lv" };
    esp_timer_create(&args, &timer);
    esp_timer_start_periodic(timer, 2000);

    wifi_init();
    ota_init();
    ui_init();

    Serial.println("[main] ready");
}

static uint32_t last_update = 0;
static uint32_t last_ota_print = 0;

void loop() {
    wifi_loop();
    ota_loop();
    check_swipe_loop();

    // Print OTA status every 30s for debugging
    if (millis() - last_ota_print > 30000) {
        last_ota_print = millis();
        Serial.printf("[OTA] listening on %s:3232\n", device_ip.c_str());
    }
    lv_timer_handler();

    if (millis() - last_update > 500) {
        last_update = millis();
        switch (active_tab) {
            case 0: screen_status_update(); break;
            case 2: screen_temp_update();   break;
            default: break;
        }
    }
}