// To compile:
// arduino-cli compile --fqbn esp32:esp32:esp32da piddle.ino
// To upload:
// arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32da
// I don't know how to set frequency and stuff easily, might need to use the Arduino IDE

// 1.2 seems stable when the ESP32 was powered by my computer
// 1.3 is not stable
#define FASTLED_OVERCLOCK 1.0

#include <arduinoFFT.h>
#include <FastLED.h>

// Some FastLED versions don't work with my ESP32-WROOM32 setup
// 3.9.6 is good, 3.9.11 I think is bad? One LED is stuck on green
static_assert(FASTLED_VERSION == 3009006);
// The parallel FastLED output only works on updated Espressif
#ifdef ESP_IDF_VERSION
static_assert(ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0));
#endif

#include "constants.hpp"
#include "spectrumAnalyzer.hpp"

void testLeds();
void blink(const int delay_ms = 500);

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
  pinMode(VOLTAGE_PIN, INPUT);

  FastLED.addLeds<WS2812B, LED_PINS[0], GRB>(leds[0], LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, LED_PINS[1], GRB>(leds[1], LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, LED_PINS[2], GRB>(leds[2], LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, LED_PINS[3], GRB>(leds[3], LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, LED_PINS[4], GRB>(leds[4], LEDS_PER_STRIP);
  FastLED.setBrightness(32);

  // That USB cord I soldered has tiny wires, shouldn't put more than 500mA through it
  if (LEDS_PER_STRIP == 60 || LEDS_PER_STRIP == 61) {
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  } else {
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 5000);
  }

  // The boot button is connected to GPIO0
  pinMode(0, INPUT);
  attachInterrupt(0, buttonInterrupt, FALLING);

  xTaskCreatePinnedToCore(
    collectSamplesFunction,
    "collectSamples",
    4000, // Stack size in words
    nullptr, // Task input parameter
    1, // Priority of the task
    &collectSamplesTask, // Task handle.
    1); // Core where the task should run

  // Test all the logic level converter LEDs
  const uint8_t tempBrightness = FastLED.getBrightness();
  FastLED.setBrightness(128);
  for (int i = 0; i < 5; ++i) {
    for (uint8_t hue = 0; hue < 240; hue += 10) {
      FastLED.clear();
      for (int strip = 0; strip < STRIP_COUNT; ++strip) {
        leds[strip][0] = CHSV(hue + strip * (255 / STRIP_COUNT), 255, 64);
      }
      FastLED.show();
      delay(1);
    }
  }
  FastLED.setBrightness(tempBrightness);

  // We need to do this last because it will preempt the setup thread that's running on core 0
  xTaskCreatePinnedToCore(
    displayLedsFunction,
    "displayLeds",
    4000, // Stack size in words
    nullptr, // Task input parameter
    1, // Priority of the task
    &displayLedsTask, // Task handle.
    0); // Core where the task should run
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
    for (int i = 0; i < 100; ++i) {
      displaySpectrumAnalyzer();
      if (Serial.available() > 0) {
        logDebug = true;
        while (Serial.available() > 0) {
          Serial.read();
        }
      }
    }
    // Keep the watchdog happy
    delay(1);
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
