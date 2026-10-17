# CYD OS — System Architecture

## Overview

CYD OS is a smartphone-style embedded operating environment for the ESP32-2432S028R (Cheap Yellow Display). It runs on an ESP32-WROOM-32 without PSRAM, targeting a 240×320 portrait TFT with resistive touch.

## Architecture Layers

```
┌─────────────────────────────────────────────────────┐
│ Layer 8: Diagnostics, OTA & Recovery                │
│   OTA updater, system diagnostics, recovery manager │
├─────────────────────────────────────────────────────┤
│ Layer 7: Persistence & Configuration                │
│   NVS/Preferences, SD filesystem, config manager    │
├─────────────────────────────────────────────────────┤
│ Layer 6: Applications                               │
│   Home, WiFi, Weather, Clock, Calculator, Notes,    │
│   Calendar, MediaHub, Settings, Diagnostics, etc.   │
├─────────────────────────────────────────────────────┤
│ Layer 5: Network & Local Services                   │
│   WiFi service, NTP, Weather API, BLE, Battery,     │
│   Notification service, Storage service              │
├─────────────────────────────────────────────────────┤
│ Layer 4: UI Framework                               │
│   LVGL theme, navigation stack, status bar,         │
│   app drawer, notification shade, widget library    │
├─────────────────────────────────────────────────────┤
│ Layer 3: Core Application Manager                   │
│   App lifecycle, event bus, task management          │
├─────────────────────────────────────────────────────┤
│ Layer 2: Hardware Abstraction Layer (HAL)           │
│   Display, Touch, SD, LED, ADC, Audio, RTC, WiFi   │
├─────────────────────────────────────────────────────┤
│ Layer 1: Boot & Recovery Manager                    │
│   Startup sequence, watchdog, reset reason handler  │
└─────────────────────────────────────────────────────┘
```

## Key Design Decisions

### Memory Management
- **No PSRAM**: All allocations from internal 520KB SRAM
- **Single LVGL draw buffer**: 240×40×2 = 19,200 bytes
- **stdlib malloc**: LVGL uses system allocator, not internal pool
- **Heap monitoring**: Warning at <20KB, critical at <8KB

### Display Architecture
- **LovyanGFX** for hardware driver (not TFT_eSPI)
- **LVGL 8.3** for UI rendering
- **Partial rendering** to minimize buffer size
- **30ms refresh** (~33 FPS target)

### SPI Bus Separation
```
HSPI (40 MHz) → TFT Display (ST7789/ILI9341)
SoftSPI (1 MHz) → Touch (XPT2046) [bit-banged]
VSPI → MicroSD Card Reader
```

### Threading Model
```
Core 1 (Arduino loop): LVGL rendering + UI logic
Core 0: WiFi stack (ESP-IDF managed)
FreeRTOS tasks: Network requests, background services
```

### UI Thread Safety
- All LVGL operations happen on the Arduino `loop()` core
- Background tasks communicate via event queue
- No direct LVGL calls from network or service tasks

## Module Dependency Graph

```
main.cpp
├── HAL Layer
│   ├── display_hal (LovyanGFX → LVGL flush)
│   ├── touch_hal (LovyanGFX touch → LVGL indev)
│   ├── sd_hal (VSPI → SD library)
│   ├── led_hal (GPIO direct)
│   └── adc_hal (ADC1 direct)
├── Core Layer (Phase C+)
│   ├── boot_manager
│   ├── app_manager
│   └── event_bus
├── UI Layer (Phase C+)
│   ├── theme
│   ├── navigation
│   └── widgets/
└── Apps (Phase D+)
    └── (app_base → individual apps)
```
