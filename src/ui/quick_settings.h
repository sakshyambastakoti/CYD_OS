/**
 * @file quick_settings.h
 * @brief Pull-down Quick Settings Control Center for CYD OS
 */

#pragma once

#include <lvgl.h>

namespace CydOS {
namespace UI {

/**
 * @brief Create quick settings panel on lv_layer_top()
 */
void quick_settings_init();

/**
 * @brief Show quick settings panel
 */
void quick_settings_show();

/**
 * @brief Hide quick settings panel
 */
void quick_settings_hide();

/**
 * @brief Toggle quick settings visibility
 */
void quick_settings_toggle();

/**
 * @brief Check if quick settings panel is visible
 */
bool quick_settings_is_visible();

} // namespace UI
} // namespace CydOS
