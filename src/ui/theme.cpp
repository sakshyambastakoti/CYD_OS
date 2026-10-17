/**
 * @file theme.cpp
 * @brief Theme implementation
 */

#include "theme.h"

namespace CydOS {
namespace UI {

lv_style_t style_screen_bg;
lv_style_t style_card;
lv_style_t style_btn;
lv_style_t style_btn_pressed;
lv_style_t style_title;
lv_style_t style_subtitle;
lv_style_t style_pill;

void theme_init() {
    // 1. Screen background
    lv_style_init(&style_screen_bg);
    lv_style_set_bg_color(&style_screen_bg, THEME_COLOR_BG);
    lv_style_set_bg_opa(&style_screen_bg, LV_OPA_COVER);

    // 2. Card surface
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, THEME_COLOR_SURFACE);
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_radius(&style_card, 10);
    lv_style_set_border_width(&style_card, 1);
    lv_style_set_border_color(&style_card, THEME_COLOR_BORDER);
    lv_style_set_pad_all(&style_card, 8);

    // 3. Button standard
    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, THEME_COLOR_SURFACE_ALT);
    lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
    lv_style_set_radius(&style_btn, 8);
    lv_style_set_border_width(&style_btn, 1);
    lv_style_set_border_color(&style_btn, THEME_COLOR_BORDER);
    lv_style_set_text_color(&style_btn, THEME_COLOR_TEXT_MAIN);
    lv_style_set_pad_all(&style_btn, 6);

    // 4. Button pressed
    lv_style_init(&style_btn_pressed);
    lv_style_set_bg_color(&style_btn_pressed, THEME_COLOR_PRIMARY);
    lv_style_set_text_color(&style_btn_pressed, THEME_COLOR_BG);

    // 5. Title text
    lv_style_init(&style_title);
    lv_style_set_text_color(&style_title, THEME_COLOR_TEXT_MAIN);
    lv_style_set_text_font(&style_title, &lv_font_montserrat_14);

    // 6. Subtitle / muted text
    lv_style_init(&style_subtitle);
    lv_style_set_text_color(&style_subtitle, THEME_COLOR_TEXT_MUTED);
    lv_style_set_text_font(&style_subtitle, &lv_font_montserrat_12);

    // 7. Pill badge
    lv_style_init(&style_pill);
    lv_style_set_bg_color(&style_pill, THEME_COLOR_SURFACE_ALT);
    lv_style_set_radius(&style_pill, LV_RADIUS_CIRCLE);
    lv_style_set_pad_hor(&style_pill, 8);
    lv_style_set_pad_ver(&style_pill, 4);
    lv_style_set_text_color(&style_pill, THEME_COLOR_PRIMARY);
    lv_style_set_text_font(&style_pill, &lv_font_montserrat_12);
}

} // namespace UI
} // namespace CydOS
