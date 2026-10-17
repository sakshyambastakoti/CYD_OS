/**
 * @file app_config.h
 * @brief Application-level constants and configuration for CYD OS
 */

#pragma once

// ==========================================================================
// CYD OS Version
// ==========================================================================
#define CYD_OS_VERSION_MAJOR    0
#define CYD_OS_VERSION_MINOR    2
#define CYD_OS_VERSION_PATCH    0
#define CYD_OS_VERSION_STRING   "0.2.0-beta"
#define CYD_OS_BUILD_PHASE      "Phase 2 - Smartphone Shell & Apps"

// ==========================================================================
// LVGL Configuration
// ==========================================================================
#define LVGL_TICK_PERIOD_MS      5
#define LVGL_TASK_PERIOD_MS      5
#define LVGL_DRAW_BUF_LINES    40      // 240 * 40 * 2 = 19,200 bytes per buffer
#define LVGL_DRAW_BUF_COUNT     1      // Single buffer (no PSRAM)

// ==========================================================================
// Task Stack Sizes (words, not bytes — multiply by 4 for bytes)
// ==========================================================================
#define TASK_STACK_UI           4096    // 16 KB — UI/LVGL task
#define TASK_STACK_NETWORK      4096    // 16 KB — WiFi/HTTP task
#define TASK_STACK_SERVICE      2048    //  8 KB — background services

// ==========================================================================
// Task Priorities (higher = more important)
// ==========================================================================
#define TASK_PRIORITY_UI           3
#define TASK_PRIORITY_NETWORK      2
#define TASK_PRIORITY_SERVICE      1

// ==========================================================================
// Display
// ==========================================================================
#define DEFAULT_BRIGHTNESS       200    // 0-255
#define MIN_BRIGHTNESS            10
#define DISPLAY_TIMEOUT_MS    30000     // 30 seconds

// ==========================================================================
// Touch
// ==========================================================================
#define TOUCH_DEBOUNCE_MS       120     // Minimum ms between touch events
#define TOUCH_POLL_PERIOD_MS     20     // Touch polling interval

// ==========================================================================
// Network
// ==========================================================================
#define WIFI_CONNECT_TIMEOUT_MS  15000
#define WIFI_SCAN_TIMEOUT_MS     10000
#define HTTP_TIMEOUT_MS           8000
#define NTP_SYNC_INTERVAL_MS  3600000   // 1 hour
#define MAX_SAVED_NETWORKS          5

// ==========================================================================
// Storage
// ==========================================================================
#define NVS_NAMESPACE           "cyd_os"
#define CONFIG_VERSION              1
#define MAX_NOTES                  50
#define MAX_NOTE_LENGTH          1024
#define MAX_ALARMS                 10
#define MAX_NOTIFICATIONS          20
#define WEATHER_CACHE_MAX_AGE_MS 1800000  // 30 minutes

// ==========================================================================
// Notification Protocol
// ==========================================================================
#define NOTIF_PROTOCOL_VERSION      1
#define NOTIF_MAX_TITLE_LEN        64
#define NOTIF_MAX_BODY_LEN        256
#define NOTIF_MAX_APP_NAME_LEN     32

// ==========================================================================
// Memory Safety
// ==========================================================================
#define HEAP_WARNING_THRESHOLD  20000   // Warn if free heap < 20 KB
#define HEAP_CRITICAL_THRESHOLD  8000   // Critical if free heap < 8 KB
