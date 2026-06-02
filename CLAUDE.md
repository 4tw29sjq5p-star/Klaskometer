# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Klaskometer** is a KLASK board game magnet tester running on an ESP32-C3 mini microcontroller. It reads magnetic field strength (Bx, By, Bz) from a SparkFun TMAG5273 breakout (40 mT range) over I2C and exposes measurements via serial JSON, a TFT display, and an embedded web dashboard over WiFi.

## PlatformIO Commands

```bash
# Build
pio run -e esp32-c3-devkitm-1

# Flash to device
pio run -t upload -e esp32-c3-devkitm-1

# Serial monitor — use PowerShell SerialPort, NOT pio device monitor
# (pio monitor background tasks exit with code 127 in this environment)
```

There are no automated tests. Manual testing: serial JSON every 200 ms, web interface at `http://klask.local` (WiFi AP: `KLASK-Tester`, password: `klask1234`).

## Architecture

All application code is in [src/main.cpp](src/main.cpp). Three output channels run in parallel:

1. **Serial** — JSON printed every 200 ms with Bx, By, Bz, magnitude, temperature.
2. **TFT display** — ST7789 172×320 driven by TFT_eSPI. Initialized in `setup()`, updated every 250 ms in `loop()` via `drawDisplay()`.
3. **Web interface** — WiFi AP + HTTP server on port 80. Full HTML5/Chart.js dashboard embedded in `main.cpp`. `/data` returns JSON; POST `/calibrate` and `/reset_cal` control offsets.

### Serial on ESP32-C3

`Serial` defaults to UART0 (not the USB port) on ESP32-C3. Two build flags in [platformio.ini](platformio.ini) route it to the Hardware USB CDC/JTAG (what appears on COM port):
```
-D ARDUINO_USB_MODE=1
-D ARDUINO_USB_CDC_ON_BOOT=1
```

### Sensor: SparkFun TMAG5273 breakout

- I2C address `0x22`, SDA pin 6, SCL pin 7
- 40 mT full-scale range on all three axes
- **Critical register map** (all offsets from chip address):
  - `0x00` DEVICE_CONFIG_1, `0x01` DEVICE_CONFIG_2, `0x02` SENSOR_CONFIG_1, `0x07` T_CONFIG
  - `0x0E`/`0x0F` MFR_ID (LSB/MSB — combined as `(mfr[1]<<8)|mfr[0]` = `0x5449`)
  - `0x10` CONV_STATUS — **must be read before result registers to latch data**
  - `0x11–0x12` X, `0x13–0x14` Y, `0x15–0x16` Z, `0x17–0x18` T (MSB first)
- **Operating mode**: `DEVICE_CONFIG_2 = 0x02` (bits [1:0] = `10` = continuous free-running). `0x04` accidentally enables I2C-trigger mode (bit 2) and standby — sensor takes no measurements.
- **Channel enable**: `SENSOR_CONFIG_1 bits [7:4]` = MAG_CH_EN. Value `3` (0x30) = X+Y+Z+T, 1 cycle. Value `7` (0x70) = NVM test mode (no measurements).

### TFT Display

Pin assignments for the ST7789 are in [include/User_Setup.h](include/User_Setup.h). Build flag `-include include/User_Setup.h` bypasses TFT_eSPI auto-detection. **Do not edit pin assignments in `main.cpp`.**

**Library patch required**: [`.pio/libdeps/esp32-c3-devkitm-1/TFT_eSPI/Processors/TFT_eSPI_ESP32_C3.h`](.pio/libdeps/esp32-c3-devkitm-1/TFT_eSPI/Processors/TFT_eSPI_ESP32_C3.h) line ~31 must use `#undef REG_SPI_BASE` (not `#ifndef`) because the SDK already defines it as returning `0` for the SPI port index used by TFT_eSPI. Without the patch, `begin_tft_write()` crashes writing to address `0x10`.

### Calibration

Offsets (`off_x`, `off_y`, `off_z`) subtract from raw sensor readings. Set via serial commands `CALIBRATE` / `RESET_CAL` or HTTP POST. Not persisted across reboots.

## Key Files

| File | Purpose |
|------|---------|
| [platformio.ini](platformio.ini) | Board, build flags (including USB CDC flags), library deps |
| [include/User_Setup.h](include/User_Setup.h) | TFT_eSPI driver + SPI pin config + font enables |
| [src/main.cpp](src/main.cpp) | Entire application logic |

## Known Issues

- **Temperature reads ~–263°C**: `T_MSB` register (0x17) returns near-zero. Root cause unclear — may be byte ordering in T result registers or a timing/format issue with the SparkFun TMAG5273 variant. Does not affect magnetic field detection.
