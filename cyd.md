# ESP32-2432S028 (CYD - Cheap Yellow Display) Complete Hardware & Display Guide

A definitive hardware reference, display driver selection guide, and pinout manual for the **ESP32-2432S028** family (popularly known as the **CYD / Cheap Yellow Display**).

---

## 📌 1. Board Overview & Variants

The **ESP32-2432S028** is an all-in-one IoT development board manufactured by Sunton and widely available on AliExpress, Amazon, and Makerfabs. It combines a 2.8" color TFT screen, touch controller, MicroSD reader, RGB LED, ambient light sensor, and audio output with an ESP32-WROOM-32 module.

### Common Variants:
| Model Code | Touch Type | Controller | Notes |
| :--- | :--- | :--- | :--- |
| **ESP32-2432S028R** | **Resistive** (Single-touch, stylus/finger pressure) | XPT2046 | Most common; yellow PCB with touch stylus included. |
| **ESP32-2432S028C** | **Capacitive** (Multi-touch, smooth glass) | CST816T or FT6236 | Higher end; smooth glass surface, responsive to light finger taps. |

### USB Port Variations:
- **Classic Model**: 1× Micro-USB port on the side (usually ILI9341 display controller).
- **Dual-Port Model**: 1× Micro-USB + 1× USB Type-C (or 2× USB Type-C). **Almost always uses the ST7789 display controller!**

---

## ⚠️ 2. The #1 Trap: Display Driver Dilemma (ST7789 vs. ILI9341)

Almost every developer working with the CYD encounters this issue: **"My display shows a scrambled half-screen, vertical colored lines, blur, or incorrect orientation."**

### Why Does This Happen?
Historically, Sunton shipped CYD boards with the **ILI9341** controller. However, newer revisions (especially 2-USB Type-C models) ship with the **ST7789** controller instead. 

Although both are 240 × 320 SPI displays, their internal register commands, RAM address offsets, and pixel interleaving differ:

| Symptom | Cause | Solution |
| :--- | :--- | :--- |
| **Half-screen blur / horizontal fuzz** | Using **ILI9341** driver on an **ST7789** board | Switch driver to **`ST7789`** |
| **Landscape orientation when set to Portrait** | Register offset mismatch (`offset_y: 0` vs `offset_y: 16`) | Use `Panel_ST7789` with `offset_y = 0` / `offset_x = 0` |
| **Mirrored / Inverted colors** | Inversion register bit (`INVON` vs `INVOFF`) | Toggle `panel.invert = false` / `true` |
| **Solid white or black screen** | Incorrect SPI pin assignment or Backlight pin not driven | Verify GPIO 21 is driven HIGH with PWM |

### How to Identify Your Display Controller:
1. **Try ST7789 first** if your board has a USB Type-C connector or was purchased recently (2024–2026).
2. If the display is crisp and properly aligned $\rightarrow$ **You have an ST7789**.
3. If ST7789 looks distorted but ILI9341 looks clean $\rightarrow$ **You have an ILI9341**.

---

## 🔌 3. Complete Pinout Reference Matrix

The CYD cleverly separates SPI buses to avoid bus conflicts between the high-speed display and the slower touch/storage peripherals:

```text
+---------------------------------------------------------------------------+
|                          ESP32-WROOM-32 (CYD)                             |
|                                                                           |
|   [ HSPI Bus - 40MHz ]          [ SoftSPI Bus - 1MHz ]     [ VSPI Bus ]   |
|   Display: ST7789 / ILI9341     Touch: XPT2046             MicroSD Slot   |
|   - SCK:  GPIO 14               - CLK:  GPIO 25            - SCK:  GPIO 18|
|   - MOSI: GPIO 13               - MOSI: GPIO 32 (T_DIN)    - MISO: GPIO 19|
|   - MISO: GPIO 12               - MISO: GPIO 39 (T_OUT)    - MOSI: GPIO 23|
|   - DC:   GPIO 2                - CS:   GPIO 33            - CS:   GPIO 5 |
|   - CS:   GPIO 15               - IRQ:  GPIO 36                           |
+---------------------------------------------------------------------------+
```

### Detailed Pin Table:

| Peripheral | Signal Function | ESP32 GPIO | Electrical Notes |
| :--- | :--- | :--- | :--- |
| **TFT LCD Glass** | SPI Clock (`SCK`) | **GPIO 14** | Hardware HSPI bus |
| | Master Out (`MOSI`) | **GPIO 13** | Hardware HSPI bus |
| | Master In (`MISO`) | **GPIO 12** | Hardware HSPI bus (optional for display) |
| | Data / Command (`DC`) | **GPIO 2** | Hardware D/C line |
| | Chip Select (`CS`) | **GPIO 15** | Active LOW |
| | Reset (`RST`) | **EN / -1** | Hardwired to ESP32 main reset pin |
| **LCD Backlight** | Backlight Control | **GPIO 21** | PWM dimming (Active HIGH, 1 kHz – 44.1 kHz) |
| **Touchscreen (XPT2046)** | Touch Clock (`T_CLK`) | **GPIO 25** | Dedicated bit-banged Software SPI |
| | Touch Data In (`T_DIN`) | **GPIO 32** | Software SPI MOSI |
| | Touch Data Out (`T_DO`) | **GPIO 39** | Software SPI MISO (Input only pin) |
| | Touch Chip Select (`T_CS`) | **GPIO 33** | Active LOW |
| | Touch Interrupt (`T_IRQ`) | **GPIO 36** | Active LOW on touch detection (Input only) |
| **MicroSD Card Slot** | SPI Clock (`SCK`) | **GPIO 18** | Hardware VSPI bus |
| | Master In (`MISO`) | **GPIO 19** | Hardware VSPI bus |
| | Master Out (`MOSI`) | **GPIO 23** | Hardware VSPI bus |
| | Chip Select (`CS`) | **GPIO 5** | Active LOW |
| **Onboard RGB LED** | Red LED | **GPIO 4** | **Active LOW** (`LOW = ON`, `HIGH = OFF`) |
| | Green LED | **GPIO 16** | **Active LOW** (`LOW = ON`, `HIGH = OFF`) |
| | Blue LED | **GPIO 17** | **Active LOW** (`LOW = ON`, `HIGH = OFF`) |
| **Light Sensor (LDR)**| Ambient Sensor | **GPIO 34** | Analog ADC1 channel 6 (Input only) |
| **Audio Output** | Speaker Amplifier | **GPIO 26** | Connected to internal audio amplifier |
| **Expansion Header (P3)**| I2C SDA / SCL | **GPIO 27 (SDA)**, **GPIO 22 (SCL)** | 3.3V logic level with GND & 3.3V pins |
| **Extended Header (CN1)**| UART TX / RX | **GPIO 1 (TX)**, **GPIO 3 (RX)** | Serial programming & debugging |
| **Extended Header (P2)**| Extended IO | **GPIO 35** (Input only), **GPIO 21** | 3.3V & GND |

---

## 💻 4. Recommended Display Library: LovyanGFX

While **TFT_eSPI** is widely known, **[LovyanGFX](https://github.com/lovyan03/LovyanGFX)** is overwhelmingly superior for the CYD for several crucial reasons:
1. **Dynamic Driver Selection**: You can switch between ST7789 and ILI9341 directly in code without modifying library header files.
2. **Dedicated Software SPI Touch Support**: Handles XPT2046 on GPIO 25/32/39/33 natively without bus collision bugs.
3. **High-Speed DMA**: 40MHz+ direct SPI DMA rendering with zero screen tearing.
4. **Built-in JPEG Decoder**: Rapidly decodes images directly from Flash (`PROGMEM`) or SD card buffers into display memory.

### Complete LovyanGFX Configuration Header for CYD:

Save this as `cyd_config.h`:

```cpp
#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// Driver IDs
#define DRIVER_ST7789    1  // Recommended for newer / 2-USB boards
#define DRIVER_ILI9341   2  // Classic CYD boards

// SELECT YOUR DRIVER HERE:
#define CYD_DISPLAY_DRIVER DRIVER_ST7789

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
            cfg.spi_host   = HSPI_HOST;     // ESP32 HSPI
            cfg.spi_mode   = 0;
            cfg.freq_write = 40000000;      // 40 MHz SPI write speed
            cfg.freq_read  = 16000000;
            cfg.pin_sclk   = 14;            // HSPI SCK
            cfg.pin_mosi   = 13;            // HSPI MOSI
            cfg.pin_miso   = 12;            // HSPI MISO
            cfg.pin_dc     = 2;             // HSPI D/C
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        // 2. Configure Display Panel
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs           = 15;      // Display CS
            cfg.pin_rst          = -1;      // Reset tied to EN
            cfg.pin_busy         = -1;
            cfg.panel_width      = 240;     // Portrait width
            cfg.panel_height     = 320;     // Portrait height
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
            cfg.pin_bl      = 21;           // Backlight pin
            cfg.invert      = false;
            cfg.freq        = 44100;        // 44.1 kHz (eliminates coil whine)
            cfg.pwm_channel = 7;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        // 4. Configure Touch Controller (XPT2046 on Bit-Banged SoftSPI)
        {
            auto cfg = _touch_instance.config();
            cfg.x_min      = 300;
            cfg.x_max      = 3900;
            cfg.y_min      = 3700;
            cfg.y_max      = 200;
            cfg.pin_int    = -1;            // Polled touch
            cfg.bus_shared = false;
            cfg.pin_sclk   = 25;            // Touch CLK
            cfg.pin_mosi   = 32;            // Touch DIN
            cfg.pin_miso   = 39;            // Touch DOUT
            cfg.pin_cs     = 33;            // Touch CS
            cfg.freq       = 1000000;       // 1 MHz SoftSPI clock
            cfg.spi_host   = -1;            // -1 enables bit-banged Software SPI
            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }

        setPanel(&_panel_instance);
    }
};

#if CYD_DISPLAY_DRIVER == DRIVER_ST7789
    using LGFX_CYD = LGFX_CYD_Template<lgfx::Panel_ST7789, 0, 16>;
#else
    using LGFX_CYD = LGFX_CYD_Template<lgfx::Panel_ILI9341, 2, 8>;
#endif
```

---

## ⚡ 5. Essential CYD Hardware Gotchas & Pro Tips

### 1. RGB Status LED is Active-LOW
The onboard RGB LED (GPIO 4, 16, 17) uses common-anode circuitry. 
- Driving a pin `LOW` (`0V`) turns the LED **ON**.
- Driving a pin `HIGH` (`3.3V`) turns the LED **OFF**.
```cpp
// Turn Green LED ON:
digitalWrite(16, LOW);
// Turn Green LED OFF:
digitalWrite(16, HIGH);
```

### 2. High-Frequency PWM Backlight Dimming
Standard 1 kHz PWM frequencies on GPIO 21 can produce an annoying, audible high-pitch coil whine on certain CYD inductors. Set your PWM frequency between **20,000 Hz and 44,100 Hz** to move the frequency beyond human hearing.

### 3. Dedicated VSPI for MicroSD Card
Never map the SD card to the display's SPI pins (GPIO 14, 13, 12). Use the hardware VSPI bus:
```cpp
SPIClass sdSPI(VSPI);
sdSPI.begin(18 /* SCK */, 19 /* MISO */, 23 /* MOSI */, 5 /* CS */);
SD.begin(5, sdSPI);
```

### 4. Input-Only GPIOs (34, 35, 36, 39)
Pins GPIO 34, 35, 36, and 39 are connected to ESP32 ADC1. They are **input-only** and do **not** have software-configurable internal pull-up or pull-down resistors.
- GPIO 34 is tied to the onboard ambient light sensor (LDR).
- GPIO 36 is tied to Touch IRQ.
- GPIO 39 is tied to Touch MISO.

### 5. Touch Screen Filtering & Debounce
The resistive XPT2046 digitizer can report sporadic jitter during finger release. Always maintain:
- Touch debounce timing: `now - lastTouchTime >= 120ms`
- Touch coordinate caching: store `touchLastX` and `touchLastY` while touching so that releasing touch doesn't register false `0` coordinates.

---

## 📦 6. PlatformIO Dependencies

To compile cleanly on PlatformIO with zero setup hassle, include the following in `platformio.ini`:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
board_build.partitions = huge_app.csv  ; Grants 3MB program flash space
lib_deps =
    lovyan03/LovyanGFX @ ^1.2.33
    Preferences
    SD
    SPI
```

---

*Document compiled for the ESP32-2432S028 CYD community & Sakshyam Bastakoti Smart Digital ID Project.*
