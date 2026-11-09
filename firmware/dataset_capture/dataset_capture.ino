// Dataset capture sketch for the ESP32-S3 dev kit (MKE-K01) + INMP441.
// Pins: WS=6, SCK=5, SD=7. Send 'R' over serial (921600 baud) to get one 1 s WAV clip.
#include <driver/i2s.h>

#define SAMPLE_RATE     16000
#define RECORD_SECONDS  1
#define SAMPLES_PER_CHUNK (SAMPLE_RATE * RECORD_SECONDS)

#define I2S_WS   6
#define I2S_SCK  5
#define I2S_SD   7
#define I2S_PORT I2S_NUM_0

int16_t buffer[SAMPLES_PER_CHUNK];

void i2s_init() {
  i2s_config_t config = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate          = SAMPLE_RATE,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 8,
    .dma_buf_len          = 512,
    .use_apll             = true,
  };
  i2s_pin_config_t pins = {
    .bck_io_num   = I2S_SCK,
    .ws_io_num    = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num  = I2S_SD
  };
  i2s_driver_install(I2S_PORT, &config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pins);
  i2s_zero_dma_buffer(I2S_PORT);
}

// 24-bit sample in the top of a 32-bit word -> shift right 14 -> clamp to int16.
// Same conversion as kws_inference.ino. Without the clamp, a loud input would wrap to the opposite sign.
static inline int16_t to_int16(int32_t raw) {
  int32_t v = raw >> 14;
  if (v > INT16_MAX) return INT16_MAX;
  if (v < INT16_MIN) return INT16_MIN;
  return (int16_t)v;
}

void recordAndSend() {
  int32_t raw[512];
  size_t bytes_read;
  int collected = 0;

  // Flush stale DMA data without stop/start (avoids a pop)
  int discard_samples = SAMPLE_RATE * 0.2; // drop the first 200 ms
  int discarded = 0;
  while (discarded < discard_samples) {
    int to_read = min(512, discard_samples - discarded);
    i2s_read(I2S_PORT, raw, to_read * 4, &bytes_read, portMAX_DELAY);
    discarded += bytes_read / 4;
  }

  // Record 1 s for real
  while (collected < SAMPLES_PER_CHUNK) {
    int to_read = min(512, SAMPLES_PER_CHUNK - collected);
    i2s_read(I2S_PORT, raw, to_read * 4, &bytes_read, portMAX_DELAY);
    int got = bytes_read / 4;
    for (int i = 0; i < got && collected < SAMPLES_PER_CHUNK; i++) {
      buffer[collected++] = to_int16(raw[i]);
    }
  }

  // Send the 44-byte WAV header
  uint32_t dataSize   = SAMPLES_PER_CHUNK * 2;
  uint32_t chunkSize  = 36 + dataSize;
  uint16_t audioFmt   = 1, nChan = 1, blockAlign = 2, bitsPerS = 16;
  uint32_t sRate      = SAMPLE_RATE, byteRate = SAMPLE_RATE * 2;
  uint32_t sub1       = 16;

  Serial.write('R'); Serial.write('I'); Serial.write('F'); Serial.write('F');
  Serial.write((uint8_t*)&chunkSize,  4);
  Serial.write('W'); Serial.write('A'); Serial.write('V'); Serial.write('E');
  Serial.write('f'); Serial.write('m'); Serial.write('t'); Serial.write(' ');
  Serial.write((uint8_t*)&sub1,       4);
  Serial.write((uint8_t*)&audioFmt,   2);
  Serial.write((uint8_t*)&nChan,      2);
  Serial.write((uint8_t*)&sRate,      4);
  Serial.write((uint8_t*)&byteRate,   4);
  Serial.write((uint8_t*)&blockAlign, 2);
  Serial.write((uint8_t*)&bitsPerS,   2);
  Serial.write('d'); Serial.write('a'); Serial.write('t'); Serial.write('a');
  Serial.write((uint8_t*)&dataSize,   4);

  // Send the PCM data in 512-byte chunks
  uint8_t* ptr = (uint8_t*)buffer;
  for (int i = 0; i < (int)dataSize; i += 512) {
    int sz = min(512, (int)dataSize - i);
    Serial.write(ptr + i, sz);
  }
  Serial.flush();
}

void setup() {
  Serial.begin(921600);
  i2s_init();
  Serial.println("READY");
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'R') {
      recordAndSend();
    }
  }
}
