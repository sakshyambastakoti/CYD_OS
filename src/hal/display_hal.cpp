/**
 * @file display_hal.cpp
 * @brief Display HAL implementation for CYD OS
 * 
 * Initializes LovyanGFX, sets up LVGL flush callback with partial rendering,
 * and manages backlight PWM.
 */

#include "display_hal.h"
#include "cyd_config.h"
#include "app_config.h"
#include "pin_config.h"
#include <lvgl.h>

// ==========================================================================
// Module state
// ==========================================================================
LGFX_CYD g_display;  // Global — accessible by touch_hal
static uint8_t  s_brightness = DEFAULT_BRIGHTNESS;
static bool     s_display_on = false;

// Accessor for other HAL modules
LGFX_CYD* display_get_instance() {
    return &g_display;
}

// LVGL draw buffer — single buffer for memory-constrained ESP32
// 240 pixels wide × LVGL_DRAW_BUF_LINES lines × 2 bytes (RGB565) = ~19.2 KB
static lv_color_t s_draw_buf[DISPLAY_WIDTH * LVGL_DRAW_BUF_LINES];
static lv_disp_draw_buf_t s_disp_draw_buf;
static lv_disp_drv_t s_disp_drv;

// ==========================================================================
// LVGL flush callback — called by LVGL when a region needs updating
// ==========================================================================
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    g_display.startWrite();
    g_display.setAddrWindow(area->x1, area->y1, w, h);
    g_display.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
    g_display.endWrite();

    lv_disp_flush_ready(drv);
}

namespace CydOS {
namespace HAL {

bool display_init() {
    // Backlight hardware pin setup
    pinMode(PIN_LCD_BL, OUTPUT);
    digitalWrite(PIN_LCD_BL, HIGH);

    // Initialize LovyanGFX
    if (!g_display.init()) {
        Serial.println("[DISP] ERROR: LovyanGFX init failed!");
        return false;
    }

    // Portrait orientation (0 = portrait)
    g_display.setRotation(0);

    // Set initial brightness (clamped to safe high-visibility range)
    if (s_brightness < 100) s_brightness = 200;
    g_display.setBrightness(s_brightness);
    s_display_on = true;

    // Clear display
    g_display.fillScreen(TFT_BLACK);

    Serial.printf("[DISP] Display initialized: %dx%d, driver=%s\n",
                  g_display.width(), g_display.height(),
                  CYD_DISPLAY_DRIVER == DRIVER_ILI9341 ? "ILI9341" : "ST7789");

    // Initialize LVGL draw buffer (single buffer, partial rendering)
    lv_disp_draw_buf_init(&s_disp_draw_buf, s_draw_buf, NULL,
                          DISPLAY_WIDTH * LVGL_DRAW_BUF_LINES);

    // Initialize LVGL display driver
    lv_disp_drv_init(&s_disp_drv);
    s_disp_drv.hor_res   = DISPLAY_WIDTH;
    s_disp_drv.ver_res   = DISPLAY_HEIGHT;
    s_disp_drv.flush_cb  = lvgl_flush_cb;
    s_disp_drv.draw_buf  = &s_disp_draw_buf;
    // Partial rendering mode (default)
    s_disp_drv.full_refresh = 0;

    lv_disp_t *disp = lv_disp_drv_register(&s_disp_drv);
    if (disp == NULL) {
        Serial.println("[DISP] ERROR: LVGL display driver registration failed!");
        return false;
    }

    Serial.println("[DISP] LVGL display driver registered");
    return true;
}

void display_set_brightness(uint8_t brightness) {
    s_brightness = brightness;
    if (s_display_on) {
        g_display.setBrightness(brightness);
    }
}

uint8_t display_get_brightness() {
    return s_brightness;
}

void display_off() {
    g_display.setBrightness(0);
    s_display_on = false;
}

void display_on() {
    g_display.setBrightness(s_brightness);
    s_display_on = true;
}

bool display_is_on() {
    return s_display_on;
}

uint16_t display_get_width() {
    return g_display.width();
}

uint16_t display_get_height() {
    return g_display.height();
}

} // namespace HAL
} // namespace CydOS
