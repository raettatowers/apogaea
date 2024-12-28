// To compile:
// arduino-cli compile --fqbn esp32:esp32:esp32da piddle.ino
// To upload:
// arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32da
// I don't know how to set frequency and stuff easily, might need to use the Arduino IDE

// 1.2 seems stable when the ESP32 was powered by my computer
// 1.3 is not stable
#define FASTLED_LED_OVERCLOCK 1.0

#include <arduinoFFT.h>
#include <FastLED.h>

// FastLED 3.9.7 causes flickering on some of my strips, so make sure we're on 3.9.6
static_assert(FASTLED_VERSION == 3009006);
// The parallel FastLED output only works on updated Espressif
#ifdef ESP_IDF_VERSION
static_assert(ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0));
#endif

#include "constants.hpp"
#include "spectrumAnalyzer.hpp"

void testLeds();
void blink(const int delay_ms = 500);

extern float minimumDivisor;
extern int startTrebleNote;
extern int additionalTrebleRange;

CRGB leds[STRIP_COUNT][LEDS_PER_STRIP];
bool logDebug = false;

TaskHandle_t collectSamplesTask;
TaskHandle_t displayLedsTask;

void buttonInterrupt() {
  // This space intentionally left blank. Kept for further expansion.
}

void setup() {
  Serial.begin(115200);

  // This had to be done first, but I think I fixed the bug that was causing problems? I don't want
  // to test if it's fixed, so I'm leaving it first now
  setupSpectrumAnalyzer();

  //analogReference(AR_DEFAULT); // Not on ESP32?
  pinMode(LED_BUILTIN, OUTPUT);

  FastLED.addLeds<WS2812B, LED_PINS[0], GRB>(leds[0], LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, LED_PINS[1], GRB>(leds[1], LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, LED_PINS[2], GRB>(leds[2], LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, LED_PINS[3], GRB>(leds[3], LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, LED_PINS[4], GRB>(leds[4], LEDS_PER_STRIP);
  FastLED.setBrightness(32);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 5000);

  // The boot button is connected to GPIO0
  pinMode(0, INPUT);
  attachInterrupt(0, buttonInterrupt, FALLING);

  // This is all configuration stuff left over from RemoteXY
  constexpr int low = 8000;
  //constexpr int high = 20500;
  constexpr int high = 40000;
  constexpr int mid = (low + high) / 2;
  minimumDivisor = mid + (-20 + 50) * (high - low) / 100;

  xTaskCreatePinnedToCore(
    collectSamplesFunction,
    "collectSamples",
    4000, // Stack size in words
    nullptr, // Task input parameter
    1, // Priority of the task
    &collectSamplesTask, // Task handle.
    1); // Core where the task should run

  xTaskCreatePinnedToCore(
    displayLedsFunction,
    "displayLeds",
    4000, // Stack size in words
    nullptr, // Task input parameter
    1, // Priority of the task
    &displayLedsTask, // Task handle.
    0); // Core where the task should run

  for (int i = 0; i < 3; ++i) {
    blink(100);
  }
}

void loop() {
  delay(10000);
}

void collectSamplesFunction(void*) {
  while (1) {
    collectSamples();
  }
}

void displayLedsFunction(void*) {
  while (1) {
    displaySpectrumAnalyzer();
    if (Serial.available() > 0) {
      logDebug = true;
      while (Serial.available() > 0) {
        Serial.read();
      }
    }
    // Keep the watchdog happy
    taskYIELD();
  }
}

void blink(const int delay_ms) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(delay_ms);
  digitalWrite(LED_BUILTIN, LOW);
  delay(delay_ms);
}

void testLeds() {
  FastLED.clear();
  for (int i = 0; i < STRIP_COUNT; ++i) {
    leds[i][5] = CRGB::Red;
    leds[i][25] = CRGB::Red;
    leds[i][50] = CRGB::Red;
    leds[i][LEDS_PER_STRIP - 10] = CRGB::Red;
  }
  FastLED.show();

  delay(20);
}
