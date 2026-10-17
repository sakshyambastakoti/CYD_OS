# CYD OS — Battery & RTC Integration Notes

## Battery Monitoring

### Hardware Requirements
The ESP32-2432S028R **does not include** an onboard battery charger, fuel gauge, or battery connector. Battery monitoring requires:

1. **External voltage divider circuit** connected to GPIO 35 (ADC1_CH7)
2. **Charging circuit** (e.g., TP4056 module) — separate from the CYD
3. **Power-path management** to safely switch between USB and battery

### Safe Wiring

> ⚠️ **NEVER connect a LiPo battery directly to GPIO pins!**
> 
> GPIO pins tolerate 0–3.3V. A charged LiPo can deliver 4.2V and damage the ESP32.

Required voltage divider:
```
Battery+ ──[ R1=100kΩ ]──┬──[ R2=100kΩ ]── GND
                          │
                      GPIO 35
```

This gives: `V_gpio = V_battery × (R2 / (R1 + R2)) = V_battery × 0.5`

- At 4.2V (full): GPIO sees 2.1V ✓
- At 3.0V (empty): GPIO sees 1.5V ✓

### ADC Calibration
ESP32 ADC is non-linear. Apply calibration:
```cpp
// Raw ADC to voltage (with voltage divider)
float voltage = (analogRead(35) / 4095.0) * 3.3 * 2.0;  // ×2 for divider
```

For accurate readings, use the ESP32 calibration API or a lookup table.

### Battery Level Estimation
LiPo discharge curve is non-linear. Approximate mapping:

| Voltage | Capacity |
|---------|----------|
| 4.20V | 100% |
| 4.06V | 90% |
| 3.98V | 80% |
| 3.92V | 70% |
| 3.87V | 60% |
| 3.82V | 50% |
| 3.79V | 40% |
| 3.77V | 30% |
| 3.74V | 20% |
| 3.68V | 10% |
| 3.50V | 5% |
| 3.00V | 0% (cutoff) |

### Software Implementation
- The `adc_hal` reads GPIO 35 with averaging (8 samples)
- `adc_battery_circuit_detected()` uses variance analysis to distinguish connected circuit from floating pin
- **Do not infer charging state from voltage alone** without a dedicated charge-status pin

### Low Battery Warning
- Configurable threshold (default: 10%)
- Display dimming at low battery
- Auto-sleep at critical level (if power management is implemented)

---

## RTC (DS3231) Integration

### Connection
The DS3231 connects via I2C on the P3 expansion header:
- SDA: GPIO 27
- SCL: GPIO 22
- VCC: 3.3V
- GND: GND

### Why an External RTC?
The ESP32's internal RTC loses time on full power loss. Without NTP (no WiFi), the clock resets to epoch. A DS3231 with coin cell battery maintains accurate time indefinitely.

### Implementation Priority
1. **NTP sync** when WiFi is available (primary time source)
2. **DS3231** when WiFi is unavailable (fallback)
3. **ESP32 internal RTC** for runtime timekeeping between syncs

### Software Support
- `rtc_hal.h/.cpp` (Phase G) will implement DS3231 read/write
- Auto-detect RTC presence on boot via I2C scan
- Sync internal RTC from DS3231 on boot
- Sync DS3231 from NTP periodically

### Current Status
Phase A does not implement RTC. The I2C bus is reserved for Phase G.
