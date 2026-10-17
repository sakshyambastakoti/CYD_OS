/**
 * @file home_screen.h
 * @brief Smartphone Home Screen & App Launcher for CYD OS
 */

#pragma once

#include <lvgl.h>

namespace CydOS {
namespace UI {

/**
 * @brief Create the Home Screen layout
 * @return Pointer to screen object
 */
lv_obj_t* home_screen_create();

/**
 * @brief Update Home Screen dynamic widgets (clock, date, stats)
 */
void home_screen_update();

} // namespace UI
} // namespace CydOS
