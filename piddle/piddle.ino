// To compile:
// arduino-cli compile --fqbn esp32:esp32:esp32doit-devkit-v1 piddle.ino
// To upload:
// arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32doit-devkit-v1 piddle.ino

#include <arduinoFFT.h>
#include <FastLED.h>

#include "constants.hpp"

void spectrumAnalyzer();
void setupSpectrumAnalyzer();

CRGB leds[LED_COUNT];

void blink(int delay_ms = 500) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(delay_ms);
  digitalWrite(LED_BUILTIN, LOW);
  delay(delay_ms);
}

void setup() {
  //analogReference(AR_DEFAULT); // Not on ESP32?
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MICROPHONE_ANALOG_PIN, INPUT);

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_COUNT);
  FastLED.setBrightness(16);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1000);

  for (int i = 0; i < 10; ++i) {
    blink(100);
  }

  setupSpectrumAnalyzer();
}

long time_ms = 0;
bool on = true;
void loop() {
  if (millis() > time_ms + 1000) {
    digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
    time_ms = millis();
    on = !on;
  }
  FastLED.clear();
  spectrumAnalyzer();
  FastLED.show();
}
