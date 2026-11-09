# Tools

## capture_audio.py

Push-to-talk recorder for the dataset. Pairs with `firmware/dataset_capture`.

```bash
pip install pyserial keyboard
python tools/capture_audio.py
```

Edit the constants at the top first:

| Constant | Meaning |
|---|---|
| `PORT` | Serial port of the capture board, for example `COM10` or `/dev/ttyUSB0` |
| `BAUD` | 921600, must match the sketch |
| `LABEL` | Class being recorded, one run per label |
| `SAVE_DIR` | Output folder, default `recordings` |

Hold SPACE to record one 1 s clip, release when done, press ESC to quit. Clips are saved as `recordings/<LABEL>/<LABEL>.NNNN.wav`. Edge Impulse reads the label from the file name up to the first `.`, so `hello.0000.wav` is labelled `hello`. Keep `LABEL` to letters, digits, space, `-` and `_`. The `keyboard` package needs administrator rights on Windows and `sudo` on Linux.
