# Hardware

Custom board: ESP32-WROOM-32UE, INMP441 I2S microphone, SSD1306 OLED, three status LEDs, three buttons, CH340C USB-UART, AMS1117-3.3 and a W25Q256JVEIQ SPI NOR flash. The board was designed in KiCad and submitted for the CDP301 course, so the design is frozen.

```
hardware/
├── schematic/   # KiCad schematic source (add your files)
├── pcb/         # layout, Gerber, drill, BOM/CPL (add your files)
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

Signal names and GPIOs for the microphone, OLED, flash, buttons and UART come from the schematic table in the report. The LED GPIOs come from the firmware (`kws_inference.ino`), which is what runs on the board.

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

## Bring-up sequence

1. Continuity and short-circuit check, then power on and measure the 3.3 V rail.
2. Flash `kws_inference` over USB-C (hold Boot, tap Reset if auto-reset does not trigger).
3. Run `tests/smoke_test.md` section B.
