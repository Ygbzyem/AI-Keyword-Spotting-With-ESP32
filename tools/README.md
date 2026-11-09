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

Hold SPACE to record one 1 s clip, release when done, press ESC to quit. Clips are saved as `recordings/<LABEL>/<LABEL>_NNNN.wav`. The `keyboard` package needs administrator rights on Windows and `sudo` on Linux.
