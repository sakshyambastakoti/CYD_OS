# CYD OS — Build & Flash Guide

## Prerequisites

- [VS Code](https://code.visualstudio.com/) with [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode)
- USB cable (Micro-USB or USB-C depending on your CYD revision)
- ESP32-2432S028R board (CYD)
- Optional: MicroSD card (FAT32 formatted)

## Quick Start

### 1. Open Project
Open the `CYD_OS/` folder in VS Code with PlatformIO installed.

### 2. Identify Your Display Driver
- **Newer boards (2024+, USB-C)**: Use `ST7789` (default)
- **Classic boards (Micro-USB only)**: Change to `ILI9341`

To change: Edit `include/cyd_config.h`, line:
```cpp
#define CYD_DISPLAY_DRIVER DRIVER_ST7789  // or DRIVER_ILI9341
```

### 3. Build
```
pio run -e cyd_os
```

### 4. Upload
```
pio run -e cyd_os -t upload
```

### 5. Monitor Serial Output
```
pio device monitor -b 115200
```

## Partition Table

The project uses a custom partition table (`partitions.csv`):
- 3 MB application space (for the full OS)
- 20 KB NVS (settings storage)
- 960 KB SPIFFS (future use)

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| LovyanGFX | ^1.2.33 | Display & touch driver |
| LVGL | ~8.3.11 | UI framework |
| ArduinoJson | ^7.0.0 | JSON parsing |

All installed automatically by PlatformIO.

## Troubleshooting

### Display shows half-screen blur
→ Wrong driver selected. Switch between `DRIVER_ST7789` and `DRIVER_ILI9341`.

### Display is solid white/black
→ Check that GPIO 21 (backlight) is connected and the board is powered correctly.

### Touch coordinates are inverted
→ Adjust `x_min`/`x_max`/`y_min`/`y_max` in `cyd_config.h` touch configuration.

### SD card not detected
→ Ensure card is FAT32 formatted and fully inserted. Check VSPI pins (18/19/23/5).

### Build fails with LVGL errors
→ Ensure `lv_conf.h` is in `include/` and `LV_CONF_INCLUDE_SIMPLE` is defined in `platformio.ini`.

## Phase A Diagnostic Output

When running Phase A firmware, you should see:
1. **Serial**: Boot banner with chip info, hardware test results, memory report
2. **Display**: Dark diagnostic dashboard showing all peripheral status
3. **LED**: Brief RGB color cycle during boot
4. **Touch**: Live coordinate display when touching the screen

Expected serial output:
```
╔══════════════════════════════════════════╗
║           CYD OS v0.1.0-alpha            ║
║     Phase A: Hardware Diagnostics        ║
║     ESP32-2432S028R (CYD)                ║
╚══════════════════════════════════════════╝

  Chip Model:    ESP32-D0WDQ6 Rev 1
  CPU Freq:      240 MHz
  Flash Size:    4096 KB
  Free Heap:     ~280000 bytes
  PSRAM Size:    0 bytes

[TEST] Display:  PASS (driver=ST7789, 240x320)
[TEST] Touch:    PASS (XPT2046, SoftSPI)
[TEST] LED:      PASS (all colors cycled)
[TEST] LDR:      PASS (raw=1234, 30%)
[TEST] Battery:  NOT CONNECTED (raw=0)
[TEST] SD Card:  MOUNTED (type=SDHC, 14919 MB, R/W=OK)
```
