# Firmware

Two Arduino sketches for two different boards. They never run together.

```
firmware/
├── kws_inference/kws_inference.ino      # custom ESP32-WROOM-32UE board
└── dataset_capture/dataset_capture.ino  # MKE-K01 ESP32-S3 dev kit
```

| Sketch | Board | Role | Needs |
|---|---|---|---|
| `kws_inference` | Custom ESP32-WROOM-32UE | Record 1 s, run MFE + CNN, show result on OLED and LEDs | Edge Impulse library, `Adafruit GFX Library`, `Adafruit SSD1306` |
| `dataset_capture` | ESP32-S3 dev kit | On serial byte `R`, stream one 1 s WAV (44-byte header + PCM) at 921600 baud | Nothing extra |

Both use the legacy `driver/i2s.h` API, 16 kHz, 32-bit I2S frames shifted right by 14 bits into 16-bit samples. Pins are `#define`s at the top of each file; the inference pins are listed in `hardware/README.md`.

## LED behavior in `kws_inference`

| State | LED | OLED |
|---|---|---|
| Idle | Red | `Ready` / `Press to speak` |
| Recording and recognizing | Yellow | `Recording...` then `Recognizing...` |
| Result | Green (red or yellow if the label is `red` or `yellow`) | Label and confidence |

The result is the top class with no rejection threshold. A threshold block exists in the source but is commented out.
