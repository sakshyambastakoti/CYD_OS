/**
 * @file nav_bar.h
 * @brief Bottom Navigation Bar for CYD OS
 */

#pragma once

#include <lvgl.h>

namespace CydOS {
namespace UI {

/**
 * @brief Initialize and mount the persistent navigation bar onto lv_layer_top()
 */
void nav_bar_init();

/**
 * @brief Set Back button visibility (hidden on Home screen)
 */
void nav_bar_set_back_visible(bool visible);

} // namespace UI
} // namespace CydOS
