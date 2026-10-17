/**
 * @file main.cpp
 * @brief CYD OS — Main Operating System Entrypoint (Phase 2)
 * 
 * Target: ESP32-2432S028R Cheap Yellow Display
 */

#include <Arduino.h>
#include <lvgl.h>

#include "pin_config.h"
#include "app_config.h"
#include "cyd_config.h"

// Hardware Abstraction Layer
#include "hal/display_hal.h"
#include "hal/touch_hal.h"
#include "hal/sd_hal.h"
#include "hal/led_hal.h"
#include "hal/adc_hal.h"
#include "hal/audio_hal.h"
#include "hal/power_hal.h"

// Core & Services
#include "core/event_bus.h"
#include "services/storage_service.h"
#include "services/time_service.h"

// UI Shell & Apps
#include "ui/theme.h"
#include "ui/status_bar.h"
#include "ui/nav_bar.h"
#include "ui/quick_settings.h"
#include "ui/app_manager.h"

using namespace CydOS;

// ==========================================================================
// Setup
// ==========================================================================
void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n==================================================");
    Serial.printf("   CYD OS v%s\n", CYD_OS_VERSION_STRING);
    Serial.println("   Target: ESP32-2432S028R (Cheap Yellow Display)");
    Serial.println("==================================================");

    // 1. Initialize LVGL Core FIRST
    Serial.println("[BOOT] Initializing LVGL core...");
    lv_init();

    // 2. Hardware: LED & ADC
    HAL::led_init();
    HAL::led_set_rgb(false, true, false); // Boot indicator: Green
    HAL::adc_init();

    // 3. Audio HAL in safe high-impedance mode
    HAL::audio_init();

    // 4. Core Event Bus & Storage (NVS)
    Serial.println("[BOOT] Initializing Storage & Settings...");
    Core::event_bus_init();
    Services::storage_init();
    auto& settings = Services::storage_get_settings();

    HAL::audio_set_volume(settings.volume);
    HAL::audio_set_muted(settings.audio_muted);

    // 5. Display & Touch via LovyanGFX
    Serial.println("[BOOT] Initializing Display...");
    if (!HAL::display_init()) {
        Serial.println("[BOOT] CRITICAL: Display initialization failed!");
        HAL::led_set_rgb(true, false, false); // Red error
        while (1) delay(1000);
    }
    HAL::display_set_brightness(settings.brightness);

    Serial.println("[BOOT] Initializing Touchscreen...");
    HAL::touch_init();

    Serial.println("[BOOT] Initializing MicroSD card...");
    HAL::sd_init();

    // 6. Power & Time Services
    HAL::power_init();
    HAL::power_set_timeout(settings.screen_timeout_sec);
    HAL::power_set_auto_brightness(settings.auto_brightness);

    Services::time_init();

    // 7. UI Design System & Shell
    Serial.println("[BOOT] Initializing UI Theme...");
    UI::theme_init();
    Serial.println("[BOOT] Initializing Quick Settings...");
    UI::quick_settings_init();
    Serial.println("[BOOT] Initializing Top Status Bar...");
    UI::status_bar_init();
    Serial.println("[BOOT] Initializing Bottom Navigation Bar...");
    UI::nav_bar_init();
    Serial.println("[BOOT] Initializing App Launcher & Home Screen...");
    UI::app_manager_init();

    // Force an immediate LVGL refresh to guarantee first frame paints to glass
    lv_timer_handler();

    // Turn off boot indicator LED
    HAL::led_set_rgb(false, false, false);

    // Friendly boot complete sound feedback
    HAL::audio_play_notification();

    Serial.printf("[BOOT] CYD OS Boot Complete! Free Heap: %u KB\n", ESP.getFreeHeap() / 1024);
    Serial.println("==================================================\n");
}

// ==========================================================================
// Main OS Loop
// ==========================================================================
void loop() {
    // 1. LVGL UI Engine Tick & Task Handler
    lv_timer_handler();

    // 2. System Power & Sleep Manager
    HAL::power_update();

    // 3. Time Service & Status Bar Updates
    Services::time_update();

    static uint32_t last_status_update = 0;
    uint32_t now = millis();
    if (now - last_status_update >= 500) {
        last_status_update = now;
        UI::status_bar_update();
    }

    // 4. Active Application Update Loop
    UI::app_manager_update();

    // 5. Yield to FreeRTOS watchdog
    delay(LVGL_TASK_PERIOD_MS);
}
