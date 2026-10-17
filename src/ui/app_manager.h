/**
 * @file app_manager.h
 * @brief Application Navigation & Lifecycle Manager for CYD OS
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

namespace CydOS {
namespace UI {

enum AppID {
    APP_NONE = 0,
    APP_HOME,
    APP_SETTINGS,
    APP_CALCULATOR,
    APP_STOPWATCH,
    APP_TORCH,
    APP_DIAGNOSTICS,
    APP_TOUCH_CAL
};

/**
 * @brief Initialize the application manager
 */
void app_manager_init();

/**
 * @brief Open an application by ID
 */
void app_manager_open(AppID app_id);

/**
 * @brief Close the current active application and return to Home
 */
void app_manager_pop();

/**
 * @brief Return directly to the Home screen
 */
void app_manager_show_home();

/**
 * @brief Get the currently active application ID
 */
AppID app_manager_get_current();

/**
 * @brief Periodic update hook for the active app
 */
void app_manager_update();

} // namespace UI
} // namespace CydOS
