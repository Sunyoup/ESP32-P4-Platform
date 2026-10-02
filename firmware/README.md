# Firmware

[中文版本](README_CN.md)

This directory contains ESP32-P4 Platform factory firmware sources and prebuilt factory firmware packages.

## Layout

- `brookesia/`: ESP-IDF firmware project.
- `factory_firmware/`: prebuilt factory firmware packages.

## Default Factory Firmware

The default prebuilt firmware is:

- `factory_firmware/ESP32-P4-Platform_Factory_Firmware_10.1-DSI-TOUCH-A_rev3.x-or-later`

It targets ESP32-P4 rev3.x or later hardware with 16 MB flash. A 7-DSI-TOUCH-A build is also provided for the existing 7-inch configuration.

## Build

Use ESP-IDF v5.5 or later:

```sh
cd firmware/brookesia
idf.py set-target esp32p4
idf.py build
```

The firmware project resolves LVGL from the ESP-IDF Component Registry and
uses the BSP copy in `brookesia/components/esp32_p4_platform`, which is
`waveshare/esp32_p4_platform` 2.0.1 plus the 7inch DSI LCD (H) panel.

### 7inch DSI LCD (H)

The [7inch DSI LCD (H)](https://www.waveshare.com/wiki/7inch_DSI_LCD_(H)) is a
1280 x 720 landscape panel that only accepts RGB888. Select it by adding
`sdkconfig.7_dsi_lcd_h.defaults` after a chip revision profile. For an
ESP32-P4-Module-DEV-KIT with rev v1.x silicon and 16 MB flash:

```sh
cd firmware/brookesia
idf.py -D "SDKCONFIG_DEFAULTS=sdkconfig.defaults;sdkconfig.rev1_3.defaults;sdkconfig.7_dsi_lcd_h.defaults" build
```

Use `sdkconfig.rev3_x.defaults` instead for rev v3.x silicon. Delete an existing
`sdkconfig` first, otherwise it overrides the defaults. This firmware project
builds with ESP-IDF v5.5.x; ESP-IDF v6 no longer ships the `json` component
that `brookesia_core` requires.

The profile sets the LCD type, RGB888 with 24-bit LVGL, and 16 MB flash. The
phone UI uses the `1280_720` stylesheet for this resolution.

The example CI workflows do not build this maintained firmware project or
regenerate prebuilt factory packages. Treat existing BIN and ZIP files as
release artifacts; update them only through an explicit firmware release and
checksum review.

The separate firmware/Brookesia product jobs build rev1_3 and rev3_x and publish
temporary 32 MiB CI artifacts. Those profile-specific artifacts are not factory
firmware and do not change the immutable rev3.x-or-later 16 MiB packages checked
into factory_firmware/.
