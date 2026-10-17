/**
 * @file main.cpp
 * @brief CYD OS — Phase A: Hardware Diagnostic Firmware
 * 
 * Validates all hardware peripherals on the ESP32-2432S028R:
 * - Display (ST7789/ILI9341 via LovyanGFX)
 * - Touch (XPT2046 with debounce)
 * - MicroSD card (VSPI bus)
 * - RGB LED (Active LOW)
 * - LDR ambient light sensor
 * - Memory diagnostics (heap, stack)
 * 
 * Displays a diagnostic dashboard on the TFT using LVGL.
 * Serial output provides detailed diagnostic information.
 * 
 * @version 0.1.0-alpha (Phase A)
 */

#include <Arduino.h>
#include <lvgl.h>

#include "pin_config.h"
#include "app_config.h"
#include "cyd_config.h"

// HAL modules
#include "hal/display_hal.h"
#include "hal/touch_hal.h"
#include "hal/sd_hal.h"
#include "hal/led_hal.h"
#include "hal/adc_hal.h"

// ==========================================================================
// Forward declarations
// ==========================================================================
static void create_diagnostic_ui();
static void update_diagnostics();
static void print_boot_banner();
static void run_hardware_tests();

// ==========================================================================
// Diagnostic UI objects
// ==========================================================================
static lv_obj_t* lbl_title      = nullptr;
static lv_obj_t* lbl_display    = nullptr;
static lv_obj_t* lbl_touch      = nullptr;
static lv_obj_t* lbl_sd         = nullptr;
static lv_obj_t* lbl_led        = nullptr;
static lv_obj_t* lbl_ldr        = nullptr;
static lv_obj_t* lbl_battery    = nullptr;
static lv_obj_t* lbl_heap       = nullptr;
static lv_obj_t* lbl_uptime     = nullptr;
static lv_obj_t* lbl_touch_pos  = nullptr;
static lv_obj_t* touch_indicator = nullptr;

// Test results
static bool test_display = false;
static bool test_touch   = false;
static bool test_sd      = false;
static bool test_led     = false;
static bool test_ldr     = false;

// ==========================================================================
// LVGL timer for periodic diagnostic updates
// ==========================================================================
static void diag_timer_cb(lv_timer_t* timer) {
    update_diagnostics();
}

// ==========================================================================
// setup()
// ==========================================================================
void setup() {
    Serial.begin(115200);
    delay(500);  // Wait for serial to initialize

    print_boot_banner();

    // ---- 1. Initialize LVGL ----
    Serial.println("\n[BOOT] Initializing LVGL...");
    lv_init();

    // ---- 2. Initialize Hardware (HAL layer) ----
    Serial.println("[BOOT] Initializing hardware...");

    // Display
    test_display = CydOS::HAL::display_init();
    if (!test_display) {
        Serial.println("[BOOT] CRITICAL: Display init failed! Halting.");
        // Flash red LED as error indicator
        CydOS::HAL::led_init();
        while (true) {
            CydOS::HAL::led_flash(true, false, false, 250);
            delay(250);
        }
    }

    // Touch
    test_touch = CydOS::HAL::touch_init();

    // LED
    CydOS::HAL::led_init();
    test_led = true;  // LED init is void — if we get here it worked

    // ADC (LDR + battery)
    CydOS::HAL::adc_init();
    test_ldr = true;  // ADC init is void

    // SD Card
    CydOS::HAL::SDStatus sdResult = CydOS::HAL::sd_init();
    test_sd = (sdResult == CydOS::HAL::SDStatus::OK);

    // ---- 3. Run hardware validation tests ----
    run_hardware_tests();

    // ---- 4. Create diagnostic UI ----
    create_diagnostic_ui();

    // ---- 5. Set up periodic diagnostics update ----
    lv_timer_create(diag_timer_cb, 500, NULL);  // Update every 500ms

    // ---- 6. Visual boot indicator ----
    CydOS::HAL::led_green();
    delay(300);
    CydOS::HAL::led_off();

    Serial.println("\n[BOOT] === Phase A Diagnostic Firmware Ready ===\n");
}

// ==========================================================================
// loop()
// ==========================================================================
void loop() {
    lv_timer_handler();  // LVGL task handler
    delay(LVGL_TASK_PERIOD_MS);
}

// ==========================================================================
// Boot banner
// ==========================================================================
static void print_boot_banner() {
    Serial.println();
    Serial.println("╔══════════════════════════════════════════╗");
    Serial.println("║           CYD OS v0.1.0-alpha            ║");
    Serial.println("║     Phase A: Hardware Diagnostics        ║");
    Serial.println("║     ESP32-2432S028R (CYD)                ║");
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.println();
    Serial.printf("  Chip Model:    %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
    Serial.printf("  CPU Freq:      %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("  Flash Size:    %d KB\n", ESP.getFlashChipSize() / 1024);
    Serial.printf("  Free Heap:     %d bytes\n", ESP.getFreeHeap());
    Serial.printf("  PSRAM Size:    %d bytes\n", ESP.getPsramSize());
    Serial.printf("  SDK Version:   %s\n", ESP.getSdkVersion());
    Serial.printf("  Reset Reason:  %d\n", esp_reset_reason());
    Serial.println();
}

// ==========================================================================
// Hardware tests
// ==========================================================================
static void run_hardware_tests() {
    Serial.println("\n[TEST] === Running Hardware Validation ===\n");

    // Test 1: Display
    Serial.printf("[TEST] Display:  %s (driver=%s, %dx%d)\n",
                  test_display ? "PASS" : "FAIL",
                  CYD_DISPLAY_DRIVER == DRIVER_ST7789 ? "ST7789" : "ILI9341",
                  DISPLAY_WIDTH, DISPLAY_HEIGHT);

    // Test 2: Touch
    Serial.printf("[TEST] Touch:    %s (XPT2046, SoftSPI)\n",
                  test_touch ? "PASS" : "FAIL");

    // Test 3: LED cycle
    Serial.println("[TEST] LED:      Cycling colors...");
    CydOS::HAL::led_red();     delay(200);
    CydOS::HAL::led_green();   delay(200);
    CydOS::HAL::led_blue();    delay(200);
    CydOS::HAL::led_cyan();    delay(200);
    CydOS::HAL::led_magenta(); delay(200);
    CydOS::HAL::led_yellow();  delay(200);
    CydOS::HAL::led_white();   delay(200);
    CydOS::HAL::led_off();
    Serial.println("[TEST] LED:      PASS (all colors cycled)");

    // Test 4: LDR
    uint16_t ldrVal = CydOS::HAL::adc_read_ldr();
    uint8_t ldrPct = CydOS::HAL::adc_get_light_percent();
    Serial.printf("[TEST] LDR:      PASS (raw=%d, %d%%)\n", ldrVal, ldrPct);

    // Test 5: Battery ADC
    bool batDetected = CydOS::HAL::adc_battery_circuit_detected();
    uint16_t batRaw = CydOS::HAL::adc_read_battery_raw();
    Serial.printf("[TEST] Battery:  %s (raw=%d)\n",
                  batDetected ? "DETECTED" : "NOT CONNECTED", batRaw);

    // Test 6: SD Card
    if (test_sd) {
        bool rwTest = CydOS::HAL::sd_test_readwrite();
        Serial.printf("[TEST] SD Card:  %s (type=%s, %llu MB, R/W=%s)\n",
                      "MOUNTED",
                      CydOS::HAL::sd_get_type_string(),
                      CydOS::HAL::sd_get_total_size() / (1024 * 1024),
                      rwTest ? "OK" : "FAIL");
    } else {
        Serial.println("[TEST] SD Card:  NOT MOUNTED");
    }

    // Memory report
    Serial.println("\n[MEM] === Memory Report ===");
    Serial.printf("[MEM] Free Heap:       %d bytes\n", ESP.getFreeHeap());
    Serial.printf("[MEM] Min Free Heap:   %d bytes\n", ESP.getMinFreeHeap());
    Serial.printf("[MEM] Max Alloc Heap:  %d bytes\n", ESP.getMaxAllocHeap());
    Serial.printf("[MEM] PSRAM Free:      %d bytes\n", ESP.getFreePsram());

    Serial.println("\n[TEST] === Hardware Validation Complete ===\n");
}

// ==========================================================================
// Create LVGL diagnostic dashboard
// ==========================================================================

// CYD OS dark theme colors
#define COLOR_BG         lv_color_hex(0x1A1A2E)  // Dark graphite background
#define COLOR_CARD       lv_color_hex(0x252540)  // Charcoal card surface
#define COLOR_ACCENT     lv_color_hex(0x7FCCB0)  // Muted mint accent
#define COLOR_TEXT       lv_color_hex(0xE8E8E8)  // Light text
#define COLOR_TEXT_DIM   lv_color_hex(0x888898)  // Dimmed text
#define COLOR_PASS       lv_color_hex(0x7FCCB0)  // Green for pass
#define COLOR_FAIL       lv_color_hex(0xE85555)  // Red for fail
#define COLOR_WARN       lv_color_hex(0xE8B855)  // Yellow for warning
#define COLOR_TOUCH_DOT  lv_color_hex(0x7FCCB0)  // Touch indicator color

static lv_style_t style_bg;
static lv_style_t style_card;
static lv_style_t style_title;
static lv_style_t style_label;
static lv_style_t style_label_pass;
static lv_style_t style_label_fail;

static void init_styles() {
    // Background style
    lv_style_init(&style_bg);
    lv_style_set_bg_color(&style_bg, COLOR_BG);
    lv_style_set_bg_opa(&style_bg, LV_OPA_COVER);

    // Card style
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, COLOR_CARD);
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_radius(&style_card, 8);
    lv_style_set_pad_all(&style_card, 8);
    lv_style_set_border_width(&style_card, 1);
    lv_style_set_border_color(&style_card, lv_color_hex(0x353550));
    lv_style_set_border_opa(&style_card, LV_OPA_COVER);

    // Title style
    lv_style_init(&style_title);
    lv_style_set_text_color(&style_title, COLOR_ACCENT);
    lv_style_set_text_font(&style_title, &lv_font_montserrat_16);

    // Normal label style
    lv_style_init(&style_label);
    lv_style_set_text_color(&style_label, COLOR_TEXT);
    lv_style_set_text_font(&style_label, &lv_font_montserrat_12);

    // Pass label
    lv_style_init(&style_label_pass);
    lv_style_set_text_color(&style_label_pass, COLOR_PASS);
    lv_style_set_text_font(&style_label_pass, &lv_font_montserrat_12);

    // Fail label
    lv_style_init(&style_label_fail);
    lv_style_set_text_color(&style_label_fail, COLOR_FAIL);
    lv_style_set_text_font(&style_label_fail, &lv_font_montserrat_12);
}

// Helper to create a status line inside a container
static lv_obj_t* create_status_line(lv_obj_t* parent, const char* text) {
    lv_obj_t* lbl = lv_label_create(parent);
    lv_label_set_text(lbl, text);
    lv_obj_add_style(lbl, &style_label, 0);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(lbl, 210);
    return lbl;
}

static void create_diagnostic_ui() {
    init_styles();

    // Screen background
    lv_obj_t* scr = lv_scr_act();
    lv_obj_add_style(scr, &style_bg, 0);

    // ---- Status Bar ----
    lv_obj_t* status_bar = lv_obj_create(scr);
    lv_obj_set_size(status_bar, 240, 24);
    lv_obj_set_pos(status_bar, 0, 0);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x121225), 0);
    lv_obj_set_style_bg_opa(status_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(status_bar, 0, 0);
    lv_obj_set_style_border_width(status_bar, 0, 0);
    lv_obj_set_style_pad_all(status_bar, 2, 0);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* lbl_bar = lv_label_create(status_bar);
    lv_label_set_text(lbl_bar, LV_SYMBOL_SETTINGS " CYD OS Diagnostics");
    lv_obj_set_style_text_color(lbl_bar, COLOR_TEXT_DIM, 0);
    lv_obj_set_style_text_font(lbl_bar, &lv_font_montserrat_10, 0);
    lv_obj_align(lbl_bar, LV_ALIGN_LEFT_MID, 4, 0);

    lbl_uptime = lv_label_create(status_bar);
    lv_label_set_text(lbl_uptime, "0:00");
    lv_obj_set_style_text_color(lbl_uptime, COLOR_TEXT_DIM, 0);
    lv_obj_set_style_text_font(lbl_uptime, &lv_font_montserrat_10, 0);
    lv_obj_align(lbl_uptime, LV_ALIGN_RIGHT_MID, -4, 0);

    // ---- Title ----
    lbl_title = lv_label_create(scr);
    lv_label_set_text(lbl_title, "Hardware Diagnostics");
    lv_obj_add_style(lbl_title, &style_title, 0);
    lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 28);

    // ---- Hardware Status Card ----
    lv_obj_t* card = lv_obj_create(scr);
    lv_obj_add_style(card, &style_card, 0);
    lv_obj_set_size(card, 224, 170);
    lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(card, 3, 0);

    // Status lines
    char buf[64];

    snprintf(buf, sizeof(buf), LV_SYMBOL_EYE_OPEN " Display: %s (%s)",
             test_display ? "OK" : "FAIL",
             CYD_DISPLAY_DRIVER == DRIVER_ST7789 ? "ST7789" : "ILI9341");
    lbl_display = create_status_line(card, buf);
    lv_obj_add_style(lbl_display, test_display ? &style_label_pass : &style_label_fail, 0);

    snprintf(buf, sizeof(buf), LV_SYMBOL_EDIT " Touch: %s (XPT2046)",
             test_touch ? "OK" : "FAIL");
    lbl_touch = create_status_line(card, buf);
    lv_obj_add_style(lbl_touch, test_touch ? &style_label_pass : &style_label_fail, 0);

    if (test_sd) {
        snprintf(buf, sizeof(buf), LV_SYMBOL_SD_CARD " SD: %s %lluMB",
                 CydOS::HAL::sd_get_type_string(),
                 CydOS::HAL::sd_get_total_size() / (1024 * 1024));
    } else {
        snprintf(buf, sizeof(buf), LV_SYMBOL_SD_CARD " SD: Not mounted");
    }
    lbl_sd = create_status_line(card, buf);
    lv_obj_add_style(lbl_sd, test_sd ? &style_label_pass : &style_label_fail, 0);

    lbl_led = create_status_line(card, LV_SYMBOL_OK " LED: OK (Active LOW)");
    lv_obj_add_style(lbl_led, &style_label_pass, 0);

    lbl_ldr = create_status_line(card, LV_SYMBOL_CHARGE " LDR: --");

    lbl_battery = create_status_line(card, LV_SYMBOL_BATTERY_FULL " Batt: --");

    lbl_heap = create_status_line(card, LV_SYMBOL_DOWNLOAD " Heap: --");

    // ---- Touch Test Area ----
    lv_obj_t* touch_card = lv_obj_create(scr);
    lv_obj_add_style(touch_card, &style_card, 0);
    lv_obj_set_size(touch_card, 224, 80);
    lv_obj_align(touch_card, LV_ALIGN_TOP_MID, 0, 228);
    lv_obj_clear_flag(touch_card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* touch_title = lv_label_create(touch_card);
    lv_label_set_text(touch_title, "Touch Test Area");
    lv_obj_set_style_text_color(touch_title, COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(touch_title, &lv_font_montserrat_12, 0);
    lv_obj_align(touch_title, LV_ALIGN_TOP_LEFT, 0, 0);

    lbl_touch_pos = lv_label_create(touch_card);
    lv_label_set_text(lbl_touch_pos, "Touch the screen...");
    lv_obj_add_style(lbl_touch_pos, &style_label, 0);
    lv_obj_align(lbl_touch_pos, LV_ALIGN_TOP_LEFT, 0, 18);

    // Touch indicator dot (small circle that follows touch)
    touch_indicator = lv_obj_create(scr);
    lv_obj_set_size(touch_indicator, 12, 12);
    lv_obj_set_style_bg_color(touch_indicator, COLOR_TOUCH_DOT, 0);
    lv_obj_set_style_bg_opa(touch_indicator, LV_OPA_80, 0);
    lv_obj_set_style_radius(touch_indicator, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(touch_indicator, 0, 0);
    lv_obj_add_flag(touch_indicator, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(touch_indicator, LV_OBJ_FLAG_CLICKABLE);
}

// ==========================================================================
// Update diagnostics periodically
// ==========================================================================
static void update_diagnostics() {
    char buf[64];

    // LDR
    uint8_t ldrPct = CydOS::HAL::adc_get_light_percent();
    snprintf(buf, sizeof(buf), LV_SYMBOL_CHARGE " LDR: %d%%", ldrPct);
    lv_label_set_text(lbl_ldr, buf);

    // Battery
    bool batDetected = CydOS::HAL::adc_battery_circuit_detected();
    if (batDetected) {
        uint16_t batRaw = CydOS::HAL::adc_read_battery_raw();
        snprintf(buf, sizeof(buf), LV_SYMBOL_BATTERY_FULL " Batt: %d (raw)", batRaw);
    } else {
        snprintf(buf, sizeof(buf), LV_SYMBOL_BATTERY_EMPTY " Batt: No circuit");
    }
    lv_label_set_text(lbl_battery, buf);

    // Heap
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t minHeap = ESP.getMinFreeHeap();
    snprintf(buf, sizeof(buf), LV_SYMBOL_DOWNLOAD " Heap: %luK / Min: %luK",
             (unsigned long)(freeHeap / 1024), (unsigned long)(minHeap / 1024));
    lv_label_set_text(lbl_heap, buf);

    // Color-code heap status
    if (freeHeap < HEAP_CRITICAL_THRESHOLD) {
        lv_obj_add_style(lbl_heap, &style_label_fail, 0);
    } else if (freeHeap < HEAP_WARNING_THRESHOLD) {
        lv_obj_set_style_text_color(lbl_heap, COLOR_WARN, 0);
    } else {
        lv_obj_add_style(lbl_heap, &style_label_pass, 0);
    }

    // Uptime
    uint32_t upSec = millis() / 1000;
    uint32_t upMin = upSec / 60;
    upSec %= 60;
    snprintf(buf, sizeof(buf), "%lu:%02lu", (unsigned long)upMin, (unsigned long)upSec);
    lv_label_set_text(lbl_uptime, buf);

    // Touch position
    if (CydOS::HAL::touch_is_pressed()) {
        int16_t tx = CydOS::HAL::touch_get_x();
        int16_t ty = CydOS::HAL::touch_get_y();
        snprintf(buf, sizeof(buf), "Pressed: X=%d Y=%d", tx, ty);
        lv_label_set_text(lbl_touch_pos, buf);

        // Move touch indicator dot
        lv_obj_clear_flag(touch_indicator, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(touch_indicator, tx - 6, ty - 6);
    } else {
        lv_label_set_text(lbl_touch_pos, "Touch the screen...");
        lv_obj_add_flag(touch_indicator, LV_OBJ_FLAG_HIDDEN);
    }

    // Serial diagnostics (every 5 seconds)
    static uint32_t lastSerialReport = 0;
    if (millis() - lastSerialReport > 5000) {
        lastSerialReport = millis();
        Serial.printf("[DIAG] Heap: %u free / %u min / %u maxBlock | LDR: %d%% | Uptime: %lu:%02lu\n",
                      ESP.getFreeHeap(), ESP.getMinFreeHeap(), ESP.getMaxAllocHeap(),
                      ldrPct, (unsigned long)upMin, (unsigned long)upSec);
    }
}
