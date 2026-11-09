# ML pipeline (Edge Impulse)

The model is trained on [Edge Impulse](https://edgeimpulse.com/) and exported as an Arduino library that `kws_inference.ino` includes as `AI_Speech_to_text_inferencing.h`.

```
raw 16 kHz audio (1 s) -> MFE -> 1D CNN (22 classes) -> INT8 / EON -> Arduino library
```

## Impulse

| Item | Value |
|---|---|
| Sample rate / length | 16 kHz, 1000 ms, 16-bit mono |
| Processing block | MFE (Mel-filterbank energy) |
| Frame length / stride | 0.025 s / 0.01 s |
| Mel filters / FFT length | 32 / 256 |
| Frequency range | 0 to 8000 Hz |
| Noise floor | -52 dB |
| Input features | 3136 (98 frames x 32 filters, reshaped to 32 columns) |

MFE is used instead of MFCC because it skips the DCT, which keeps more spectral detail for the convolution layers and costs less on the MCU.

## Network

```
Input (3136)
└── Reshape (32 columns)
    ├── Conv1D 16 filters, kernel 3 -> Dropout 0.25
    ├── Conv1D 32 filters, kernel 3 -> Dropout 0.25
    └── Flatten -> Dense 128 -> Dropout 0.25 -> Output (22 classes)
```

| Training setting | Value |
|---|---|
| Training cycles | 200 |
| Learning rate | 0.0001 |
| Processor | CPU |
| Augmentation | Add noise: low, mask time bands: low, mask frequency bands: low, warp time axis: off |

## Results (from the Edge Impulse project)

| Metric | Value |
|---|---|
| Validation accuracy | 91.2 % |
| Validation loss | 0.36 |
| Lowest F1 per class | red 0.82 |
| Highest F1 per class | six 0.99, seven 0.98, eight 0.98 |

## Deployment and size

Settings: Arduino library, EON Compiler. The numbers below are Edge Impulse estimates for ESP32-WROOM-32UE, not measurements on the board.

| | MFE | Classifier | Total |
|---|---|---|---|
| INT8 latency | 361 ms | 11 ms | 372 ms |
| INT8 RAM / flash | | 9.3 KB / 136.1 KB | 16.8 KB RAM |
| float32 latency | 361 ms | 91 ms | 452 ms |
| float32 RAM / flash | | 26.6 KB / 445.7 KB | 26.6 KB RAM |

## Reproduce

1. Create an Edge Impulse project, upload the clips (see `dataset/README.md`), keep the 80/20 split. The label is taken from the file name up to the first `.` (`hello.0000.wav` becomes `hello`), or set it in the uploader.
2. Create the impulse with the table above, train, check the confusion matrix.
3. Deployment tab: Arduino library, EON Compiler, Quantized (int8), Build.
4. Put the downloaded `.zip` in `ml/edge-impulse-model/` and install it (see root README, Build and Flash). The exact export used for the results above is attached to the GitHub Release `v1.0.0` (details in `ml/edge-impulse-model/README.md`).

If your Edge Impulse project has a different name, the generated header name changes. Rename the project to `AI_Speech_to_text` or edit the `#include` in `kws_inference.ino`.
