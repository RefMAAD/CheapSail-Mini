# CheapSail Mini — Beta 1.0r OTA

A wireless Klipper touchscreen controller built on the ESP32-2432S022C  
(CYD — Cheap Yellow Display).

**Author:** [RefMAAD](https://github.com/RefMAAD)  
**Version:** Beta 1.0r OTA  
**License:** MIT

> If you build on this project or publish a derivative work, a mention of the original author would be appreciated:  
> *Based on [CheapSail Mini](https://github.com/RefMAAD/CheapSail-Mini) by RefMAAD*  
> This is a courtesy request, not a legal requirement.

---

## Features

- **WiFi** connection to Moonraker API via WebSocket
- **OTA firmware updates** — flash wirelessly after first USB install, no cable needed
- **5 tabs** navigated by swipe left/right
  - **Status** — temperatures, print progress, pause/resume/cancel
  - **Move** — manual axis jog, homing
  - **Temp** — heater presets (PLA/ABS), filament load/unload
  - **Calib.** — bed mesh, screw tilt, Z-offset, feeler gauge
  - **IS** — Input Shaper test and auto-calibration
- **Battery indicator** via IP5306 I2C
- **Long-press protection** on all action buttons
- **CheapSail Mini splash screen** with fade-in animation on boot

## Hardware

| Component | Details |
|-----------|---------|
| Board | ESP32-2432S022C (CYD 2.2") |
| Display | ST7789 240×320 parallel 8-bit |
| Touch | CST816S/CST820 capacitive I2C |
| Battery mgmt | IP5306 I2C |
| Connectivity | WiFi 802.11 b/g/n |

## Requirements

- [PlatformIO](https://platformio.org/) (VS Code extension recommended)
- Klipper + [Moonraker](https://github.com/Arksine/moonraker) on your printer
- A compatible 3.7V LiPo battery (optional)

## Setup

1. Clone this repository
2. Open the folder in VS Code with PlatformIO installed
3. Edit `platformio.ini` — fill in your WiFi credentials and Moonraker IP:
   ```ini
   -D WIFI_SSID=\"YourNetworkName\"
   -D WIFI_PASS=\"YourPassword\"
   -D MOONRAKER_HOST=\"192.168.x.x\"
   ```
4. Build once (✓ button) — this downloads all libraries
5. Copy `include/lv_conf.h` to `.pio/libdeps/esp32dev/lvgl/lv_conf.h`
6. Build and flash via USB (→ button)

## OTA updates

After the first USB flash, all future updates can be done wirelessly:

1. Find the device IP on the Status screen
2. Add to `platformio.ini`:
   ```ini
   upload_protocol = espota
   upload_port     = 192.168.x.x
   upload_flags    = --auth=cheapsail
   ```
3. Click upload (→) as normal — no USB cable needed

## Moonraker configuration

Add your local network to `moonraker.conf` trusted clients:

```ini
[authorization]
trusted_clients:
    192.168.0.0/16
```

## Navigation

| Gesture | Action |
|---------|--------|
| Swipe left | Next tab |
| Swipe right | Previous tab |
| Long press | Execute action (safety) |

## Project structure

```
CheapSail-Mini/
├── platformio.ini
├── include/
│   ├── lv_conf.h       — LVGL configuration (must copy to libdeps after clean build)
│   └── config.h        — Project-wide settings and version
└── src/
    ├── main.cpp         — LGFX, LVGL, swipe nav, splash screen
    ├── wifi_comms.h/.cpp — Moonraker WebSocket
    ├── battery.h        — IP5306 battery level
    ├── ota.h            — OTA firmware update with progress UI
    └── screens/
        ├── screen_status.cpp
        ├── screen_move.cpp
        ├── screen_temp.cpp
        ├── screen_calibrate.cpp
        └── screen_shaper.cpp
```

## Known limitations

- Touch coordinates may be slightly offset depending on board revision
- IP5306 battery reading shows USB icon when powered without battery
- `FEELER_ZERO` macro requires a custom Klipper macro definition
- `FULL_CALIBRATION` macro requires a custom Klipper macro definition

## Contributing

Pull requests welcome! Ideas for future development:
- Touch offset calibration UI
- Bed mesh visualisation
- Print thumbnail display
- Fan speed control
- CheapSail Midi (3.5" ESP32-3248S035C) — coming soon

## License

MIT — free to use, modify and distribute.
