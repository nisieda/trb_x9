# trb_x9

ZMK config for a 9-key + trackball keyboard, targeting the Seeed Studio XIAO BLE (`xiao_ble`) board directly (no separate carrier/MCU board).

## Hardware

- Board: `xiao_ble`
- Shield: `trb_x9`
- Trackball sensor: PixArt PAW3222 (via [sekigon-gonnoc/zmk-driver-paw3222](https://github.com/sekigon-gonnoc/zmk-driver-paw3222))
- Key matrix: 3x3 (9 keys), wired to XIAO BLE pins D0-D5 (see `config/boards/shields/trb_x9/trb_x9.overlay` for the exact pinout)
- Trackball SPI: D8-D10 (board default), CS on D6, IRQ on D7

## Features

- 15-degree step trackball rotation (`&rotate_step`, custom input processor in `src/`)
- Hold-to-scroll on the M3 key (`&mb3_scroll`)

## Repo layout

This repo follows the standard ZMK "zmk-config" structure: everything under `config/` is the user config (keymap, west.yml, shield definition), while `CMakeLists.txt` / `Kconfig` / `src/` / `dts/` at the repo root make this repo double as a west module contributing the custom `zmk,input-processor-rotate` and `zmk,behavior-rotate-step` drivers.
