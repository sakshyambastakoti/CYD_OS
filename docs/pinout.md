# CYD OS — Hardware Pinout Reference

## ESP32-2432S028R GPIO Map

| GPIO | Function | Direction | Bus | Notes |
|------|----------|-----------|-----|-------|
| **2** | LCD DC (Data/Command) | Output | HSPI | - |
| **4** | LED Red | Output | GPIO | **Active LOW** |
| **5** | SD Card CS | Output | VSPI | Active LOW |
| **12** | LCD MISO | Input | HSPI | Optional for display |
| **13** | LCD MOSI | Output | HSPI | - |
| **14** | LCD SCK | Output | HSPI | 40 MHz |
| **15** | LCD CS | Output | HSPI | Active LOW |
| **16** | LED Green | Output | GPIO | **Active LOW** |
| **17** | LED Blue | Output | GPIO | **Active LOW** |
| **18** | SD SCK | Output | VSPI | - |
| **19** | SD MISO | Input | VSPI | - |
| **21** | LCD Backlight | Output | PWM | Active HIGH, 44.1kHz |
| **22** | I2C SCL | Bidir | I2C | P3 expansion header |
| **23** | SD MOSI | Output | VSPI | - |
| **25** | Touch CLK | Output | SoftSPI | Bit-banged, 1 MHz |
| **26** | Speaker | Output | DAC | Via amplifier |
| **27** | I2C SDA | Bidir | I2C | P3 expansion header |
| **32** | Touch DIN (MOSI) | Output | SoftSPI | - |
| **33** | Touch CS | Output | SoftSPI | Active LOW |
| **34** | LDR (Light Sensor) | **Input Only** | ADC1_CH6 | No pull-up available |
| **35** | Battery ADC (ext.) | **Input Only** | ADC1_CH7 | No pull-up available |
| **36** | Touch IRQ | **Input Only** | GPIO | Active LOW |
| **39** | Touch DOUT (MISO) | **Input Only** | SoftSPI | - |

## SPI Bus Allocation

```
┌──────────────────────────────────────────────────┐
│ HSPI (Host SPI 2) — 40 MHz                      │
│ ├── TFT Display (ST7789 / ILI9341)              │
│ └── Pins: SCK=14, MOSI=13, MISO=12, DC=2, CS=15│
├──────────────────────────────────────────────────┤
│ SoftSPI (Bit-banged) — 1 MHz                    │
│ ├── Touch Controller (XPT2046)                   │
│ └── Pins: CLK=25, DIN=32, DOUT=39, CS=33        │
├──────────────────────────────────────────────────┤
│ VSPI (Host SPI 3)                                │
│ ├── MicroSD Card Reader                          │
│ └── Pins: SCK=18, MISO=19, MOSI=23, CS=5        │
└──────────────────────────────────────────────────┘
```

## Important Constraints

1. **Input-only pins** (34, 35, 36, 39): Cannot output, no internal pull-up
2. **ADC2 unavailable** when WiFi is active — use ADC1 only (GPIO 32-39)
3. **GPIO 2**: Also connected to onboard LED on some ESP32 modules; used as LCD DC
4. **LCD RST**: Tied to ESP32 EN pin (hardware reset only)
5. **Backlight PWM**: Use 20-44.1 kHz to avoid audible coil whine
