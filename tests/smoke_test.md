# Smoke tests

Run these in order after flashing. Each step has an observable pass condition. Results are recorded by hand in the table at the bottom; nothing here has been auto-verified.

## A. Dataset capture (`firmware/dataset_capture`, ESP32-S3 dev kit)

| # | Action | Pass condition |
|---|---|---|
| A1 | Flash `dataset_capture`, open a serial monitor at 921600 baud, reset the board | Prints `READY` |
| A2 | Close the monitor, run `python tools/capture_audio.py`, hold SPACE and say a word | Prints `[OK] recordings/<LABEL>/<LABEL>_0000.wav` |
| A3 | `python tests/check_wav.py recordings/` | Every file reports `OK` (a `WARN` for silence is acceptable only for `noise`) |
| A4 | Open one file in an audio player | The word is audible, with no loud pop at the start |

## B. On-device inference (`firmware/kws_inference`, custom ESP32-WROOM-32UE board)

| # | Action | Pass condition |
|---|---|---|
| B1 | Power over USB-C, serial monitor at 115200 baud | Prints `Ready - press the button to record 1 s`; OLED shows `Ready` / `Press to speak`; red LED on |
| B2 | Press and hold the Record button | Yellow LED on; OLED shows `Recording...`, then `Recognizing...`; serial prints the same |
| B3 | Say a trained keyword (for example `book`) while holding | Serial prints `>>> <label> (<confidence>%)`; OLED shows label and percentage; green LED on |
| B4 | Say `red` / `yellow` / `green` | The matching LED lights (red/yellow/green) |
| B5 | Release the button, wait 1.5 s | OLED returns to `Ready`; red LED on |
| B6 | Stay silent while pressing | The top class is still shown, because the firmware applies no rejection threshold. Record the label and confidence you get |

If the OLED stays blank, try `OLED_ADDR 0x3D`. If serial prints `ERR: I2S init failed`, check the three I2S pins against `hardware/README.md`.

## Result log (fill in per board)

| Date | Board | Steps passed | Notes |
|---|---|---|---|
| | | | |
