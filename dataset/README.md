# Dataset

All audio was recorded with the project's own microphone. Raw `.wav` files are not committed (size, and everyone's voice and room differ), so record your own set for best accuracy.

```
Capture chain
INMP441 -> ESP32-S3 dev kit (MKE-K01, dataset_capture.ino) --USB serial 921600--> PC (tools/capture_audio.py) -> recordings/<label>/<label>.NNNN.wav
```

## Labels (22 classes, noise included)

```
bed  bird  book  bye  cat  dog  door  eight  five  four  green
hello  nine  noise  one  red  seven  six  ten  three  two  yellow
```

`noise` is the negative class: record background sound and non-keyword speech for it.

## Recording protocol

1. Flash `firmware/dataset_capture` to the ESP32-S3 dev kit.
2. Set `PORT` and `LABEL` at the top of `tools/capture_audio.py`, then run it (see `tools/README.md`).
3. Hold SPACE, speak, release. Each take is a 1 s, 16 kHz, 16-bit mono WAV.
4. Vary distance, speed and background noise between takes.
5. Check the batch: `python tests/check_wav.py recordings/`.

The sketch drops the first 200 ms after each trigger to avoid a start-up pop and sends exactly 16000 samples.

## Statistics (Edge Impulse project)

| Item | Value |
|---|---|
| Total audio | 2 h 55 m 8 s |
| Training / test samples | 8,375 / 2,132 (80 % / 20 %) |
| Real recordings per class | about 120 to 200 |
| Synthetic samples per class | about 100 to 150, generated with OpenAI text-to-speech |
| Format | 16 kHz, 16-bit PCM, 1000 ms |
