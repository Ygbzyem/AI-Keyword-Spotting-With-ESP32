#define EIDSP_QUANTIZE_FILTERBANK 0
#include <AI_Speech_to_text_inferencing.h>
#include "driver/i2s.h"
#include <esp_log.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---- I2S pins: match the custom ESP32-WROOM-32UE schematic; avoid GPIO6-11 (internal flash) ----
#define I2S_WS   17
#define I2S_SCK  4
#define I2S_SD   16
#define I2S_PORT I2S_NUM_0

#define OLED_SDA 22   // change if your board wires I2C differently
#define OLED_SCL 21   // change if your board wires I2C differently
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C   // most 0.96" modules use 0x3C; try 0x3D if the screen stays blank

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---- Record button and LEDs: GPIO34 is input-only with no internal pull, the board adds an external 10k pull-up ----
#define BUTTON_PIN 34
#define LED_GREEN  19
#define LED_RED    18
#define LED_YELLOW 5

static int16_t *record_buffer = nullptr;
static bool debug_nn = false;

static bool i2s_init() {
    i2s_config_t i2s_config = {
        .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate          = EI_CLASSIFIER_FREQUENCY,
        .bits_per_sample      = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count        = 8,
        .dma_buf_len          = 512,
        .use_apll             = true,
        .tx_desc_auto_clear   = false,
        .fixed_mclk           = 0
    };
    i2s_pin_config_t pin_config = {
        .bck_io_num   = I2S_SCK,
        .ws_io_num    = I2S_WS,
        .data_out_num = -1,
        .data_in_num  = I2S_SD,
    };
    if (i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL) != ESP_OK) return false;
    if (i2s_set_pin(I2S_PORT, &pin_config) != ESP_OK) return false;
    i2s_zero_dma_buffer(I2S_PORT);
    return true;
}

// Blocking capture of exactly 16000 samples = 1 s, the same length used for training
static void record_one_second() {

    // Flush the first 200 ms of stale DMA data
    int32_t dummy[512];
    size_t dummy_read;
    int discard = EI_CLASSIFIER_FREQUENCY * 0.2;
    int discarded = 0;
    while (discarded < discard) {
        i2s_read(I2S_PORT, dummy, sizeof(dummy), &dummy_read, portMAX_DELAY);
        discarded += dummy_read / sizeof(int32_t);
    }

    const uint32_t total_samples = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
    uint32_t collected = 0;
    const uint32_t chunk_samples = 512;
    int32_t raw32[chunk_samples];

    while (collected < total_samples) {
        size_t bytes_read = 0;
        i2s_read(I2S_PORT, (void*)raw32, chunk_samples * sizeof(int32_t),
                  &bytes_read, portMAX_DELAY);
        int samples = bytes_read / sizeof(int32_t);
        for (int i = 0; i < samples && collected < total_samples; i++) {
            record_buffer[collected++] = (int16_t)(raw32[i] >> 14);
        }
    }

    int64_t dc = 0;
    for (uint32_t i = 0; i < total_samples; i++) dc += record_buffer[i];
    dc /= total_samples;
    for (uint32_t i = 0; i < total_samples; i++) {
        record_buffer[i] = (int16_t)(record_buffer[i] - dc);
    }
}

static int get_signal_data(size_t offset, size_t length, float *out_ptr) {
    numpy::int16_to_float(&record_buffer[offset], out_ptr, length);
    return 0;
}

static void oled_show(const char* line1, const char* line2 = "") {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.println(line1);
    if (strlen(line2) > 0) {
        display.setTextSize(1);
        display.setCursor(0, 40);
        display.println(line2);
    }
    display.display();
}

static void led_set(bool r, bool y, bool g) {
    digitalWrite(LED_RED,    r ? HIGH : LOW);
    digitalWrite(LED_YELLOW, y ? HIGH : LOW);
    digitalWrite(LED_GREEN,  g ? HIGH : LOW);
}

void setup() {
    Serial.begin(115200);
    esp_log_level_set("*", ESP_LOG_NONE);
    pinMode(BUTTON_PIN, INPUT);   // pressed = LOW

    record_buffer = (int16_t*)malloc(EI_CLASSIFIER_RAW_SAMPLE_COUNT * sizeof(int16_t));
    if (!record_buffer) { Serial.println("ERR: not enough RAM"); return; }

    if (!i2s_init()) { Serial.println("ERR: I2S init failed"); return; }

    Serial.println("Ready - press the button to record 1 s");

    Wire.begin(OLED_SDA, OLED_SCL);
if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("ERR: OLED not found");
} else {
    oled_show("Ready", "Press to speak");
    }

    pinMode(LED_RED, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    led_set(true, false, false);
}

void loop() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        delay(30); // debounce
        if (digitalRead(BUTTON_PIN) != LOW) return;

        // Recording + recognizing: yellow LED
        led_set(false, true, false);
        Serial.println("Recording...");
        oled_show("Recording...");
        record_one_second();

        Serial.println("Recognizing...");
        oled_show("Recognizing...");

        signal_t signal;
        signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
        signal.get_data = &get_signal_data;

        ei_impulse_result_t result = { 0 };
        EI_IMPULSE_ERROR r = run_classifier(&signal, &result, debug_nn);
        if (r != EI_IMPULSE_OK) {
            Serial.printf("Classifier error: %d\n", r);
            oled_show("Error!", "Try again");
            led_set(true, false, false); // on error, go back to idle
            while (digitalRead(BUTTON_PIN) == LOW) delay(10);
            return;
        }

        float max_val = 0;
        int max_idx = 0;
        for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
            if (result.classification[i].value > max_val) {
                max_val = result.classification[i].value;
                max_idx = i;
            }
        }
        const char* label = result.classification[max_idx].label;

   /*  if (max_val < 0.85 || strcmp(label, "noise") == 0) {
        Serial.println(">>> Not recognized / noise, skipped");
        oled_show("Try again", "Speak clearly");
        led_set(true, false, false);
        while (digitalRead(BUTTON_PIN) == LOW) delay(10);
        return;
}   */

        Serial.printf(">>> %s (%.1f%%)\n", label, max_val * 100);

        char percent_str[16];
        snprintf(percent_str, sizeof(percent_str), "%.1f%%", max_val * 100);
        oled_show(label, percent_str);

        // Show result: green LED first
        led_set(false, false, true);

        // If the label is a color name, switch to the matching LED (turns green off)
        if (strcmp(label, "red") == 0) {
            led_set(true, false, false);
        } else if (strcmp(label, "yellow") == 0) {
            led_set(false, true, false);
        } else if (strcmp(label, "green") == 0) {
            led_set(false, false, true); // keep green
        }

        while (digitalRead(BUTTON_PIN) == LOW) delay(10); // wait for button release

        delay(1500); // hold the result for 1.5 s
        oled_show("Ready", "Press to speak");
        led_set(true, false, false);   // back to idle: red LED
    }
}
