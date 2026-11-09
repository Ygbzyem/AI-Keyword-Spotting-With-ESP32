# Hardware

Custom board: ESP32-WROOM-32UE, INMP441 I2S microphone, SSD1306 OLED, three status LEDs, three buttons, CH340C USB-UART, AMS1117-3.3 and a W25Q256JVEIQ SPI NOR flash. The board was designed in KiCad and submitted for the CDP301 course, so the design is frozen.

```
hardware/
├── schematic/AI_Keyword_Spotting.kicad_sch   # KiCad schematic
├── pcb/AI_Keyword_Spotting.kicad_pcb         # KiCad layout of the fabricated board
└── README.md
```

## Bill of materials (functional)

| Part | Role | Note |
|---|---|---|
| ESP32-WROOM-32UE | Main controller | 240 MHz dual core, 520 KB SRAM, external antenna connector |
| INMP441 | I2S MEMS microphone | 24-bit digital output, no external ADC |
| SSD1306 0.96" OLED | Result display | I2C, address 0x3C |
| AMS1117-3.3 | LDO regulator | 5 V USB to 3.3 V, up to 800 mA |
| CH340C | USB-to-UART bridge | Flashing and serial monitor, auto-reset via DTR/RTS transistors |
| W25Q256JVEIQ | SPI NOR flash (256 Mbit) | On its own SPI pins, see below |
| 3 x LED (red, yellow, green) | Status | Each through a 10 k resistor |
| 3 x push button | Reset, Boot, Record | 10 k pull-ups, 0.1 uF on Reset and Boot |

## Pin mapping

Signal names and GPIOs for the microphone, OLED, flash, buttons and UART come from the schematic table in the report. The LED GPIOs come from the firmware (`kws_inference.ino`), which is what runs on the board. In the demo photo of the idle state (`docs/images/10-demo-standby.jpg`) the red LED is lit while the firmware drives IO18, which agrees with the table below. The schematic names IO5 as red and IO18 as yellow, so its LED labels are swapped relative to the board as built (see Known discrepancies).

| Signal | GPIO | Peripheral | Note |
|---|---|---|---|
| SD | IO16 | INMP441 | I2S data |
| WS | IO17 | INMP441 | I2S word select |
| SCK | IO4 | INMP441 | I2S bit clock |
| L/R | GND | INMP441 | Selects the left channel |
| SCL | IO21 | SSD1306 | I2C clock |
| SDA | IO22 | SSD1306 | I2C data |
| CS | IO23 | W25Q256JVEIQ | SPI chip select |
| SCK_F | IO25 | W25Q256JVEIQ | SPI clock |
| MOSI | IO26 | W25Q256JVEIQ | |
| MISO | IO27 | W25Q256JVEIQ | |
| LED red | IO18 | Status LED | Idle / "red" result |
| LED yellow | IO5 | Status LED | Recording and recognizing / "yellow" result |
| LED green | IO19 | Status LED | Result |
| Boot | IO0 | Button | 10 k pull-up |
| Record | IO34 | Button | Input-only pin, 10 k external pull-up, active-low |
| Reset | EN | Button | 10 k pull-up |
| TXD0 / RXD0 | IO1 / IO3 | CH340C | To the PC through USB-C |

The dataset capture sketch runs on a different board, the MKE-K01 ESP32-S3 dev kit, with the microphone on WS=6, SCK=5, SD=7.

## Known discrepancies

The board was fabricated and submitted for the course, so these are documented, not changed. The PCB file describes the fabricated board; the schematic file does not match it exactly.

| # | Item | Detail |
|---|---|---|
| 1 | LED names | Schematic nets: IO5 = red, IO18 = yellow, IO19 = green. Firmware and the idle photo: IO18 = red, IO5 = yellow, IO19 = green. Use the table above. |
| 2 | Schematic vs PCB | The schematic (KiCad 10 file) has `J3` (INMP441 header) and `R6` (0 ohm) that the PCB (KiCad 9 file) does not have; the PCB has two 1x3 headers `J4` and `J5` that the schematic lacks. Do not run "Update PCB from Schematic" on these files. |
| 3 | Missing footprints | `Q1`, `Q2`, `R11` to `R14`, `C12` and `J3` have no footprint assigned in the schematic. |
| 4 | Flash part value | Schematic `U2` is labelled W25Q128JVE (128 Mbit); the part on the BOM and in the report is W25Q256JVEIQ (256 Mbit). The flash is not used by the firmware. |
| 5 | Not run | No ERC or DRC result is recorded for these files. |

## Bring-up sequence

1. Continuity and short-circuit check, then power on and measure the 3.3 V rail.
2. Flash `kws_inference` over USB-C (hold Boot, tap Reset if auto-reset does not trigger).
3. Run `tests/smoke_test.md` section B.
