/**
 * @file theme.h
 * @brief Modern Dark Smartphone UI Theme for CYD OS
 */

#pragma once

#include <lvgl.h>

namespace CydOS {
namespace UI {

// Color definitions (LVGL 16-bit color macros)
#define THEME_COLOR_BG           lv_color_hex(0x10111A)
#define THEME_COLOR_SURFACE      lv_color_hex(0x1C1D2A)
#define THEME_COLOR_SURFACE_ALT  lv_color_hex(0x27293D)
#define THEME_COLOR_PRIMARY      lv_color_hex(0x4ECCA3)
#define THEME_COLOR_SECONDARY    lv_color_hex(0x00ADB5)
#define THEME_COLOR_ACCENT_ORANGE lv_color_hex(0xFF8A5B)
#define THEME_COLOR_TEXT_MAIN    lv_color_hex(0xF5F6FA)
#define THEME_COLOR_TEXT_MUTED   lv_color_hex(0x8F94A6)
#define THEME_COLOR_BORDER       lv_color_hex(0x2F324A)
#define THEME_COLOR_SUCCESS      lv_color_hex(0x2ECC71)
#define THEME_COLOR_WARN         lv_color_hex(0xF39C12)
#define THEME_COLOR_DANGER       lv_color_hex(0xE74C3C)

// Shared styles
extern lv_style_t style_screen_bg;
extern lv_style_t style_card;
extern lv_style_t style_btn;
extern lv_style_t style_btn_pressed;
extern lv_style_t style_title;
extern lv_style_t style_subtitle;
extern lv_style_t style_pill;

/**
 * @brief Initialize all theme styles
 */
void theme_init();

} // namespace UI
} // namespace CydOS
