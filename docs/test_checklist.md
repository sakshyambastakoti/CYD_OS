# CYD OS — Hardware Test Checklist

## Phase A: Hardware Validation

### Pre-Test Setup
- [ ] Board connected via USB
- [ ] Serial monitor running at 115200 baud
- [ ] Phase A firmware uploaded successfully
- [ ] SD card inserted (FAT32, optional)

### Test 1: Boot & Serial
- [ ] Boot banner appears on serial monitor
- [ ] Chip model shows "ESP32-D0WDQ6" (or similar)
- [ ] CPU frequency shows 240 MHz
- [ ] Flash size shows ~4096 KB
- [ ] Free heap shows ~270-300 KB
- [ ] PSRAM shows 0 bytes
- [ ] Reset reason is displayed

### Test 2: Display
- [ ] Display turns on (backlight illuminates)
- [ ] Screen shows diagnostic UI (not white/black)
- [ ] Colors appear correct (dark background, mint accents)
- [ ] No half-screen blur or color bands
- [ ] Text is readable and properly oriented (portrait)
- [ ] If display shows issues → switch driver in `cyd_config.h`

### Test 3: Touch
- [ ] Serial reports "Touch: PASS"
- [ ] Touching screen updates coordinate display
- [ ] Coordinates track finger position accurately
- [ ] Touch indicator dot follows finger
- [ ] No phantom touches when not touching
- [ ] Coordinates reset properly on release
- [ ] Touch works in all four corners

### Test 4: RGB LED
- [ ] LED cycles through colors during boot
- [ ] Red, Green, Blue, Cyan, Magenta, Yellow, White visible
- [ ] LED turns off after boot sequence
- [ ] Green flash indicates successful boot

### Test 5: LDR (Light Sensor)
- [ ] LDR reading displayed on diagnostic screen
- [ ] Value changes when covering/uncovering sensor
- [ ] Percentage increases with more light
- [ ] Readings are stable (not jumping wildly)

### Test 6: MicroSD Card
- [ ] SD card type detected (SD, SDHC, etc.)
- [ ] Card size displayed correctly
- [ ] Read/write test passes
- [ ] "Not mounted" shown if no card inserted
- [ ] No crash on card removal

### Test 7: Battery ADC
- [ ] "No circuit" shown if no battery wiring
- [ ] If battery circuit installed: raw ADC value displayed
- [ ] Value is stable (not fluctuating wildly)

### Test 8: Memory
- [ ] Free heap shown on diagnostic screen
- [ ] Min free heap tracking active
- [ ] No memory warnings at boot
- [ ] Heap stays stable over 5+ minutes

### Test 9: Uptime
- [ ] Uptime counter incrementing on status bar
- [ ] Serial diagnostics print every 5 seconds

---

## Known Issues (Phase A)

| Issue | Status | Notes |
|-------|--------|-------|
| Touch calibration not yet interactive | Planned for Phase B | Using LovyanGFX defaults |
| No WiFi in Phase A | By design | Phase E |
| No app navigation | By design | Phase C |
| `delay()` in LED flash is blocking | Acceptable for diagnostics | Will use async in production |

---

## Pass Criteria for Phase A

All Phase A tests must pass before proceeding to Phase B:
1. ✅ Display renders correctly in portrait 240×320
2. ✅ Touch reports coordinates when pressed
3. ✅ RGB LED cycles all colors
4. ✅ LDR reads ambient light
5. ✅ SD card mounts and passes R/W test (if inserted)
6. ✅ Heap remains stable over 5-minute run
7. ✅ No crashes or watchdog resets
