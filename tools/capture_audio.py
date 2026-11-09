# capture_audio.py - push-to-talk WAV recorder for the dataset_capture sketch
import serial
import os
import keyboard
import time

PORT     = "COM10"
BAUD     = 921600
LABEL    = "green5"
SAVE_DIR = "recordings"

os.makedirs(f"{SAVE_DIR}/{LABEL}", exist_ok=True)

ser = serial.Serial(PORT, BAUD, timeout=10)
time.sleep(2)
ser.reset_input_buffer()

count   = 0
is_held = False

print("=" * 40)
print(f"  Label : {LABEL}")
print(f"  Save to: {SAVE_DIR}/{LABEL}/")
print("=" * 40)
print("  Hold SPACE -> speak -> release SPACE")
print("  Press ESC to quit\n")

def record_one():
    global count

    # Send the trigger command to the ESP32
    ser.write(b'R')
    ser.flush()

    # Read the 44-byte WAV header
    header = ser.read(44)
    if len(header) < 44 or header[0:4] != b'RIFF':
        print("[WARN] Bad header, skipping...")
        ser.reset_input_buffer()  # flush the buffer, then retry
        return

    data_size = int.from_bytes(header[40:44], 'little')
    data = ser.read(data_size)

    if len(data) < data_size:
        print("[WARN] Incomplete data, skipping...")
        return

    filename = f"{SAVE_DIR}/{LABEL}/{LABEL}_{count:04d}.wav"
    with open(filename, 'wb') as f:
        f.write(header)
        f.write(data)

    count += 1
    print(f"[OK] {filename}")
    time.sleep(0.3)  # short pause between takes

try:
    while True:
        if keyboard.is_pressed('esc'):
            print(f"\n[DONE] Saved {count} file(s)")
            break

        if keyboard.is_pressed('space'):
            if not is_held:
                is_held = True
                print("[REC] Recording...", end='\r')
                record_one()
        else:
            if is_held:
                is_held = False
                print("[IDLE] Ready          ", end='\r')

        time.sleep(0.01)

except KeyboardInterrupt:
    print(f"\n[DONE] Saved {count} file(s)")

finally:
    ser.close()
