/**
 * @file status_bar.h
 * @brief Persistent Top Status Bar for CYD OS
 */

#pragma once

#include <lvgl.h>

namespace CydOS {
namespace UI {

/**
 * @brief Create and mount the persistent status bar onto lv_layer_top()
 */
void status_bar_init();

/**
 * @brief Update status bar widgets (clock, WiFi, battery, SD)
 */
void status_bar_update();

} // namespace UI
} // namespace CydOS
