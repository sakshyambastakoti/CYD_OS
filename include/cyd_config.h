/**
 * @file cyd_config.h
 * @brief LovyanGFX display and touch configuration for ESP32-2432S028R
 * 
 * Supports both ST7789 (newer boards) and ILI9341 (classic boards).
 * Default: ST7789 — change CYD_DISPLAY_DRIVER if your board uses ILI9341.
 * 
 * Configuration derived from verified CYD hardware reference (cyd.md).
 */

#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "pin_config.h"

// ==========================================================================
// Driver Selection
// ==========================================================================
#define DRIVER_ST7789    1  // Newer boards, USB-C models (2024+)
#define DRIVER_ILI9341   2  // Classic CYD boards, single Micro-USB

// Default to ST7789 for recent boards — change if display looks scrambled
#ifndef CYD_DISPLAY_DRIVER
#define CYD_DISPLAY_DRIVER DRIVER_ST7789
#endif

// ==========================================================================
// LovyanGFX Device Template
// ==========================================================================
template <typename TPanel, int RotOffset = 0, int ExtraOffset = 0>
class LGFX_CYD_Template : public lgfx::LGFX_Device {
    lgfx::Panel_Device* _panel_ptr = nullptr;
    lgfx::Bus_SPI       _bus_instance;
    TPanel              _panel_instance;
    lgfx::Light_PWM     _light_instance;
    lgfx::Touch_XPT2046 _touch_instance;

public:
    LGFX_CYD_Template() {
        // 1. Configure HSPI Bus for Display
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host   = HSPI_HOST;
            cfg.spi_mode   = 0;
            cfg.freq_write = 40000000;       // 40 MHz SPI write speed
            cfg.freq_read  = 16000000;
            cfg.pin_sclk   = PIN_LCD_SCK;
            cfg.pin_mosi   = PIN_LCD_MOSI;
            cfg.pin_miso   = PIN_LCD_MISO;
            cfg.pin_dc     = PIN_LCD_DC;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        // 2. Configure Display Panel
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs           = PIN_LCD_CS;
            cfg.pin_rst          = PIN_LCD_RST;   // -1 = tied to EN
            cfg.pin_busy         = -1;
            cfg.panel_width      = DISPLAY_WIDTH;  // 240
            cfg.panel_height     = DISPLAY_HEIGHT; // 320
            cfg.offset_x         = 0;
            cfg.offset_y         = ExtraOffset;
            cfg.offset_rotation  = RotOffset;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits  = 1;
            cfg.readable         = false;
            cfg.invert           = false;
            cfg.rgb_order        = false;
            cfg.dlen_16bit       = false;
            cfg.bus_shared       = false;
            _panel_instance.config(cfg);
        }

        // 3. Configure Backlight (PWM on GPIO 21)
        {
            auto cfg = _light_instance.config();
            cfg.pin_bl      = PIN_LCD_BL;
            cfg.invert      = false;
            cfg.freq        = BL_PWM_FREQ;        // 44.1 kHz
            cfg.pwm_channel = BL_PWM_CHANNEL;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        // 4. Configure Touch Controller (XPT2046 on Software SPI)
        {
            auto cfg = _touch_instance.config();
            cfg.x_min      = 300;
            cfg.x_max      = 3900;
            cfg.y_min      = 3700;
            cfg.y_max      = 200;
            cfg.pin_int    = -1;           // Polled mode (no IRQ)
            cfg.bus_shared = false;
            cfg.pin_sclk   = PIN_TOUCH_CLK;
            cfg.pin_mosi   = PIN_TOUCH_DIN;
            cfg.pin_miso   = PIN_TOUCH_DOUT;
            cfg.pin_cs     = PIN_TOUCH_CS;
            cfg.freq       = 1000000;      // 1 MHz SoftSPI clock
            cfg.spi_host   = -1;           // -1 = bit-banged Software SPI
            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }

        setPanel(&_panel_instance);
    }
};

// ==========================================================================
// Driver-specific instantiation
// ==========================================================================
#if CYD_DISPLAY_DRIVER == DRIVER_ST7789
    // ST7789: RotOffset=0, ExtraOffset=16 (RAM address offset)
    class LGFX_CYD : public LGFX_CYD_Template<lgfx::Panel_ST7789, 0, 16> {};
#elif CYD_DISPLAY_DRIVER == DRIVER_ILI9341
    // ILI9341: RotOffset=2, ExtraOffset=8
    class LGFX_CYD : public LGFX_CYD_Template<lgfx::Panel_ILI9341, 2, 8> {};
#else
    #error "CYD_DISPLAY_DRIVER must be DRIVER_ST7789 or DRIVER_ILI9341"
#endif
