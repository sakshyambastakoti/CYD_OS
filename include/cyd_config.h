/**
 * @file cyd_config.h
 * @brief LovyanGFX display and touch configuration for ESP32-2432S028R
 * 
 * Supports ST7789, ILI9341, and ST7796 display panels.
 * Default: ST7789 (verified on user's dual-USB / CYD hardware).
 */

#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "pin_config.h"

// ==========================================================================
// Driver Selection IDs
// ==========================================================================
#define DRIVER_ST7789     1  // Newer boards, USB-C models (2024+) - ACTIVE
#define DRIVER_ILI9341    2  // Sunton classic CYD with rotation offset 2
#define DRIVER_ILI9341_0  3  // Classic CYD with rotation offset 0
#define DRIVER_ST7796     4  // 3.5" or alternate CYD panel

// Set ST7789 as active default
#ifndef CYD_DISPLAY_DRIVER
#define CYD_DISPLAY_DRIVER DRIVER_ST7789
#endif

// ==========================================================================
// LovyanGFX Device Template (Matched with verified hardware profile)
// ==========================================================================
template <typename TPanel, int OffsetRot = 0, int DummyRead = 16>
class LGFX_CYD_Template : public lgfx::LGFX_Device {
    TPanel              _panel_instance;
    lgfx::Bus_SPI       _bus_instance;
    lgfx::Light_PWM     _light_instance;
    lgfx::Touch_XPT2046 _touch_instance;

public:
    LGFX_CYD_Template(void) {
        // 1. Configure Hardware SPI Bus for Display (HSPI on GPIO 14, 13, 12, 2)
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host    = HSPI_HOST;     // Hardware SPI host for display
            cfg.spi_mode    = 0;
            cfg.freq_write  = 40000000;      // 40 MHz high-speed rendering
            cfg.freq_read   = 16000000;
            cfg.spi_3wire   = false;
            cfg.dma_channel = 1;             // Enable DMA channel 1 for fast rendering
            cfg.pin_sclk    = PIN_LCD_SCK;   // GPIO 14
            cfg.pin_mosi    = PIN_LCD_MOSI;  // GPIO 13
            cfg.pin_miso    = PIN_LCD_MISO;  // GPIO 12
            cfg.pin_dc      = PIN_LCD_DC;    // GPIO 2
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        // 2. Configure Display Panel (Dimensions, Offsets, Color Inversion)
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs           = PIN_LCD_CS;     // GPIO 15
            cfg.pin_rst          = PIN_LCD_RST;    // -1: Connected to EN
            cfg.pin_busy         = -1;
            cfg.panel_width      = DISPLAY_WIDTH;  // 240
            cfg.panel_height     = DISPLAY_HEIGHT; // 320
            cfg.offset_x         = 0;
            cfg.offset_y         = 0;              // 0 offset (verified)
            cfg.offset_rotation  = OffsetRot;
            cfg.dummy_read_pixel = DummyRead;      // 16 for ST7789, 8 for ILI
            cfg.dummy_read_bits  = 1;
            cfg.readable         = false;
            cfg.invert           = false;
            cfg.rgb_order        = false;
            cfg.dlen_16bit       = false;
            cfg.bus_shared       = false;
            _panel_instance.config(cfg);
        }

        // 3. Configure Backlight PWM Control
        {
            auto cfg = _light_instance.config();
            cfg.pin_bl      = PIN_LCD_BL;     // GPIO 21
            cfg.invert      = false;
            cfg.freq        = BL_PWM_FREQ;    // 44.1 kHz audible-free PWM frequency
            cfg.pwm_channel = BL_PWM_CHANNEL; // 7
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        // 4. Configure XPT2046 Resistive Touch Controller (SoftSPI)
        {
            auto cfg = _touch_instance.config();
            cfg.x_min      = 300;
            cfg.x_max      = 3900;
            cfg.y_min      = 3700;
            cfg.y_max      = 200;
            cfg.pin_int    = -1;             // Polled touch
            cfg.bus_shared = false;          // Dedicated touchscreen bus
            cfg.pin_sclk   = PIN_TOUCH_CLK;  // GPIO 25 (T_CLK)
            cfg.pin_mosi   = PIN_TOUCH_DIN;  // GPIO 32 (T_DIN)
            cfg.pin_miso   = PIN_TOUCH_DOUT; // GPIO 39 (T_OUT)
            cfg.pin_cs     = PIN_TOUCH_CS;   // GPIO 33 (T_CS)
            cfg.freq       = 1000000;        // 1 MHz SPI clock for stable touch sampling
            cfg.spi_host   = -1;             // -1: Use bit-banged Software SPI
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
    // ST7789: Verified working profile for user's CYD board
    class LGFX_CYD : public LGFX_CYD_Template<lgfx::Panel_ST7789, 0, 16> {};
#elif CYD_DISPLAY_DRIVER == DRIVER_ILI9341
    // ILI9341 with rotation offset 2 (Sunton classic CYD)
    class LGFX_CYD : public LGFX_CYD_Template<lgfx::Panel_ILI9341, 2, 8> {};
#elif CYD_DISPLAY_DRIVER == DRIVER_ILI9341_0
    // ILI9341 with rotation offset 0
    class LGFX_CYD : public LGFX_CYD_Template<lgfx::Panel_ILI9341, 0, 8> {};
#elif CYD_DISPLAY_DRIVER == DRIVER_ST7796
    // ST7796 panel
    class LGFX_CYD : public LGFX_CYD_Template<lgfx::Panel_ST7796, 0, 8> {};
#else
    // Fallback default: ST7789
    class LGFX_CYD : public LGFX_CYD_Template<lgfx::Panel_ST7789, 0, 16> {};
#endif
