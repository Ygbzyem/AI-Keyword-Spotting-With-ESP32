#!/usr/bin/env python3
"""Check that recordings match the format the Edge Impulse project expects.

Usage:
    python tests/check_wav.py recordings/            # every .wav below a folder
    python tests/check_wav.py a.wav b.wav            # explicit files

FAIL (exit code 1): not PCM, not mono, not 16 kHz, not 16-bit, not exactly 1 s.
WARN (exit code 0): nearly silent, clipped, or with a large DC offset. Silence is normal for the `noise` label.
The device removes the DC offset of every 1 s window before inference but the capture sketch does not, so the offset
reported here shows how far the training clips differ from what the model sees on the board.
"""
import sys
import wave
from pathlib import Path
import array

RATE, WIDTH, CHANNELS, FRAMES = 16000, 2, 1, 16000
SILENT_PEAK = 200        # heuristic: peak below this is probably a missed take
CLIP_FRACTION = 0.01     # heuristic: >1 % of samples at full scale is clipping
DC_LIMIT = 500           # heuristic: |mean| above this (about -36 dBFS) is worth a look


def check(path):
    fails, warns = [], []
    try:
        with wave.open(str(path), "rb") as w:
            ch, width, rate, frames = w.getnchannels(), w.getsampwidth(), w.getframerate(), w.getnframes()
            pcm = w.readframes(frames)
    except (wave.Error, EOFError) as e:
        return [f"not a readable PCM WAV ({e!r})"], []
    if ch != CHANNELS:
        fails.append(f"channels={ch}, expected {CHANNELS}")
    if width != WIDTH:
        fails.append(f"sample width={width * 8} bit, expected {WIDTH * 8}")
    if rate != RATE:
        fails.append(f"sample rate={rate}, expected {RATE}")
    if frames != FRAMES:
        fails.append(f"frames={frames}, expected {FRAMES} (1 s)")
    if not fails:
        s = array.array("h")
        s.frombytes(pcm)
        if sys.byteorder == "big":
            s.byteswap()
        peak = max(abs(v) for v in s)
        if peak < SILENT_PEAK:
            warns.append(f"nearly silent (peak={peak})")
        if sum(1 for v in s if abs(v) >= 32767) / len(s) > CLIP_FRACTION:
            warns.append("clipping (>1 % of samples at full scale)")
        dc = sum(s) / len(s)
        if abs(dc) > DC_LIMIT:
            warns.append(f"DC offset {dc:.0f} (device removes it, capture does not)")
    return fails, warns


def main(args):
    files = []
    for a in args:
        p = Path(a)
        files += sorted(p.rglob("*.wav")) if p.is_dir() else [p]
    if not files:
        print("no .wav files given")
        return 2
    bad = 0
    for f in files:
        fails, warns = check(f)
        status = "FAIL" if fails else ("WARN" if warns else "OK  ")
        print(f"{status} {f}" + "".join(f"\n       - {m}" for m in fails + warns))
        bad += bool(fails)
    print(f"\n{len(files)} file(s), {bad} failed")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
