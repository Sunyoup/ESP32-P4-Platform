# 固件

[English Version](README.md)

本目录包含 ESP32-P4 Platform 出厂固件源码和预编译出厂固件包。

## 目录

- `brookesia/`：ESP-IDF 固件工程。
- `factory_firmware/`：预编译出厂固件包。

## 默认出厂固件

默认预编译固件为：

- `factory_firmware/ESP32-P4-Platform_Factory_Firmware_10.1-DSI-TOUCH-A_rev3.x-or-later`

该固件面向使用 16 MB Flash 的 ESP32-P4 rev3.x 或更新硬件。目录中也提供现有
7 英寸配置使用的 7-DSI-TOUCH-A 构建。

## 构建

使用 ESP-IDF v5.5 或更新版本：

```sh
cd firmware/brookesia
idf.py set-target esp32p4
idf.py build
```

固件工程从 ESP-IDF Component Registry 解析 LVGL，并使用
`brookesia/components/esp32_p4_platform` 中的 BSP 副本，即
`waveshare/esp32_p4_platform` 2.0.1 加上 7inch DSI LCD (H) 面板。

### 7inch DSI LCD (H)

[7inch DSI LCD (H)](https://www.waveshare.com/wiki/7inch_DSI_LCD_(H)) 是
1280 x 720 横屏面板，只接受 RGB888。在芯片版本 profile 之后追加
`sdkconfig.7_dsi_lcd_h.defaults` 即可选择它。对于 rev v1.x 芯片、16 MB Flash 的
ESP32-P4-Module-DEV-KIT：

```sh
cd firmware/brookesia
idf.py -D "SDKCONFIG_DEFAULTS=sdkconfig.defaults;sdkconfig.rev1_3.defaults;sdkconfig.7_dsi_lcd_h.defaults" build
```

rev v3.x 芯片请改用 `sdkconfig.rev3_x.defaults`。请先删除已有的 `sdkconfig`，
否则它会覆盖 defaults。本固件工程使用 ESP-IDF v5.5.x 构建；ESP-IDF v6 不再提供
`brookesia_core` 依赖的 `json` 组件。

该 profile 设置 LCD 类型、RGB888 与 24 位 LVGL，以及 16 MB Flash。手机 UI 在此
分辨率下使用 `1280_720` 样式表。

示例 CI 工作流不会构建这个维护固件工程，也不会重新生成预编译出厂包。现有
BIN 和 ZIP 文件属于发布制品；只有在明确的固件发布和校验值复核流程中才应更新。

独立的 firmware/Brookesia 产品 job 会构建 rev1_3 和 rev3_x，并发布临时的
32 MiB CI artifact。这些 profile 专属 artifact 不是出厂固件，也不会修改已提交到
factory_firmware/ 的不可变 rev3.x-or-later 16 MiB package。
