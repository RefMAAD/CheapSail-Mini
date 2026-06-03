/**
 * lv_conf.h — LVGL 8.x configuration
 * ESP32-2432S022C + ST7789 240x320
 *
 * IMPORTANT: After each clean build, copy this file to:
 *   .pio/libdeps/esp32dev/lvgl/lv_conf.h
 */

#if 1

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* ── Color depth ────────────────────────────────────────────────────────────── */
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 1

/* ── Memory ─────────────────────────────────────────────────────────────────── */
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE   (48 * 1024U)

/* ── HAL ────────────────────────────────────────────────────────────────────── */
#define LV_TICK_CUSTOM 0
#define LV_DPI_DEF 130

/* ── Drawing ────────────────────────────────────────────────────────────────── */
#define LV_DRAW_COMPLEX 1
#define LV_SHADOW_CACHE_SIZE 0
#define LV_CIRCLE_CACHE_SIZE 4
#define LV_IMG_CACHE_DEF_SIZE 0

/* ── GPU disabled ───────────────────────────────────────────────────────────── */
#define LV_USE_GPU_STM32_DMA2D 0
#define LV_USE_GPU_SWM341_DSPY 0
#define LV_USE_GPU_NXP_PXP 0
#define LV_USE_GPU_NXP_VG_LITE 0
#define LV_USE_GPU_SDL 0

/* ── Logging ────────────────────────────────────────────────────────────────── */
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
#define LV_LOG_PRINTF 1
#define LV_LOG_TRACE_MEM 0
#define LV_LOG_TRACE_TIMER 0
#define LV_LOG_TRACE_INDEV 0
#define LV_LOG_TRACE_DISP_REFR 0
#define LV_LOG_TRACE_EVENT 0
#define LV_LOG_TRACE_OBJ_CREATE 0
#define LV_LOG_TRACE_LAYOUT 0
#define LV_LOG_TRACE_ANIM 0

/* ── Debug ──────────────────────────────────────────────────────────────────── */
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ 0
#define LV_ASSERT_HANDLER_INCLUDE <stdint.h>
#define LV_ASSERT_HANDLER while(1);

/* ── Fonts ──────────────────────────────────────────────────────────────────── */
#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0
#define LV_FONT_UNSCII_8  0
#define LV_FONT_UNSCII_16 0
#define LV_FONT_DEFAULT &lv_font_montserrat_12

#define LV_TXT_ENC LV_TXT_ENC_UTF8
#define LV_TXT_BREAK_CHARS " ,.;:-_"
#define LV_TXT_LINE_BREAK_LONG_LEN 0
#define LV_TXT_COLOR_CMD "#"
#define LV_USE_BIDI 0
#define LV_USE_ARABIC_PERSIAN_CHARS 0

/* ── Animation ──────────────────────────────────────────────────────────────── */
#define LV_USE_ANIMATION 1

/* ── Widgets ────────────────────────────────────────────────────────────────── */
#define LV_USE_ARC        1
#define LV_USE_BAR        1
#define LV_USE_BTN        1
#define LV_USE_BTNMATRIX  1   /* required by lv_calendar */
#define LV_USE_CANVAS     0
#define LV_USE_CHECKBOX   0
#define LV_USE_DROPDOWN   1   /* required by lv_calendar_header_dropdown */
#define LV_USE_IMG        1   /* required by animimg dependency */
#define LV_USE_LABEL      1
#define LV_LABEL_TEXT_SELECTION 0
#define LV_LABEL_LONG_TXT_HINT  0
#define LV_USE_LINE       0
#define LV_USE_ROLLER     0
#define LV_USE_SLIDER     0
#define LV_USE_SWITCH     0
#define LV_USE_TEXTAREA   1
#define LV_TEXTAREA_DEF_PWD_SHOW_TIME 1500
#define LV_USE_TABLE      0
#define LV_USE_CHART      0
#define LV_USE_COLORWHEEL 0
#define LV_USE_IMGBTN     0
#define LV_USE_KEYBOARD   0
#define LV_USE_LED        0
#define LV_USE_LIST       0
#define LV_USE_MENU       0
#define LV_USE_METER      0
#define LV_USE_MSGBOX     0
#define LV_USE_SPAN       0
#define LV_USE_SPINBOX    0
#define LV_USE_SPINNER    0
#define LV_USE_TABVIEW    0
#define LV_USE_TILEVIEW   0
#define LV_USE_WIN        0

/* ── Themes ─────────────────────────────────────────────────────────────────── */
#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 1
#define LV_THEME_DEFAULT_GROW 0
#define LV_THEME_DEFAULT_TRANSITION_TIME 80
#define LV_USE_THEME_SIMPLE 0
#define LV_USE_THEME_MONO 0

/* ── Layout ─────────────────────────────────────────────────────────────────── */
#define LV_USE_FLEX 1
#define LV_USE_GRID 0

/* ── Third party disabled ───────────────────────────────────────────────────── */
#define LV_USE_FS_STDIO 0
#define LV_USE_FS_POSIX 0
#define LV_USE_FS_WIN32 0
#define LV_USE_FS_FATFS 0
#define LV_USE_PNG 0
#define LV_USE_BMP 0
#define LV_USE_JPG 0
#define LV_USE_GIF 0
#define LV_USE_QRCODE 0
#define LV_USE_FREETYPE 0
#define LV_USE_RLOTTIE 0
#define LV_USE_FFMPEG 0

/* ── Demos disabled ─────────────────────────────────────────────────────────── */
#define LV_USE_DEMO_WIDGETS            0
#define LV_USE_DEMO_KEYPAD_AND_ENCODER 0
#define LV_USE_DEMO_BENCHMARK          0
#define LV_USE_DEMO_STRESS             0
#define LV_USE_DEMO_MUSIC              0

#endif /* LV_CONF_H */
#endif /* if 1 */
