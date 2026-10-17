/**
 * @file pin_config.h
 * @brief GPIO pin definitions for ESP32-2432S028R (CYD)
 * 
 * All pin assignments verified against the CYD hardware reference.
 * Do NOT change these unless you have a different board revision.
 */

#pragma once

// ==========================================================================
// Display — HSPI Bus (40 MHz)
// ==========================================================================
#define PIN_LCD_SCK         14      // HSPI CLK
#define PIN_LCD_MOSI        13      // HSPI MOSI
#define PIN_LCD_MISO        12      // HSPI MISO
#define PIN_LCD_DC           2      // Data/Command
#define PIN_LCD_CS          15      // Chip Select (Active LOW)
#define PIN_LCD_RST         -1      // Tied to ESP32 EN (hardware reset)
#define PIN_LCD_BL          21      // Backlight (Active HIGH, PWM capable)

// ==========================================================================
// Touch — Software SPI (1 MHz, Bit-Banged)
// Controller: XPT2046 (Resistive)
// ==========================================================================
#define PIN_TOUCH_CLK       25      // Software SPI CLK
#define PIN_TOUCH_DIN       32      // Software SPI MOSI (T_DIN)
#define PIN_TOUCH_DOUT      39      // Software SPI MISO (T_DO) — INPUT ONLY
#define PIN_TOUCH_CS        33      // Chip Select (Active LOW)
#define PIN_TOUCH_IRQ       36      // Interrupt (Active LOW) — INPUT ONLY

// ==========================================================================
// MicroSD Card — VSPI Bus
// ==========================================================================
#define PIN_SD_SCK          18      // VSPI CLK
#define PIN_SD_MISO         19      // VSPI MISO
#define PIN_SD_MOSI         23      // VSPI MOSI
#define PIN_SD_CS            5      // Chip Select (Active LOW)

// ==========================================================================
// RGB LED — Active LOW (LOW = ON, HIGH = OFF)
// ==========================================================================
#define PIN_LED_RED          4
#define PIN_LED_GREEN       16
#define PIN_LED_BLUE        17

// ==========================================================================
// Ambient Light Sensor (LDR)
// ==========================================================================
#define PIN_LDR             34      // ADC1_CH6 — INPUT ONLY

// ==========================================================================
// Audio Output
// ==========================================================================
#define PIN_SPEAKER         26      // Via onboard amplifier

// ==========================================================================
// I2C Expansion Header (P3) — for RTC, sensors, etc.
// ==========================================================================
#define PIN_I2C_SDA         27
#define PIN_I2C_SCL         22

// ==========================================================================
// UART (CN1 Header)
// ==========================================================================
#define PIN_UART_TX          1
#define PIN_UART_RX          3

// ==========================================================================
// Optional Battery ADC (external circuit required)
// GPIO 35 is available on P2 header, input-only, no pull-up
// ==========================================================================
#define PIN_BATTERY_ADC     35      // ADC1_CH7 — INPUT ONLY

// ==========================================================================
// Display Constants
// ==========================================================================
#define DISPLAY_WIDTH       240
#define DISPLAY_HEIGHT      320

// ==========================================================================
// Backlight PWM Configuration
// ==========================================================================
#define BL_PWM_CHANNEL       7
#define BL_PWM_FREQ      44100     // 44.1 kHz — above human hearing
#define BL_PWM_RESOLUTION    8     // 8-bit (0–255)
