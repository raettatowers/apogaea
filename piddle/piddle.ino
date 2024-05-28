// To compile:
// arduino-cli compile --fqbn esp32:esp32:esp32da piddle.ino
// To upload:
// arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32da
// I don't know how to set frequency and stuff easily, might need to use the Arduino IDE

#include <arduinoFFT.h>
#include <FastLED.h>

#include "constants.hpp"

void spectrumAnalyzer();
void setupSpectrumAnalyzer();

CRGB leds[STRIP_COUNT][LEDS_PER_STRIP];

void blink(const int delay_ms = 500) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(delay_ms);
  digitalWrite(LED_BUILTIN, LOW);
  delay(delay_ms);
}

void buttonInterrupt() {
  // This space intentionally left blank. Kept for further expansion.
}

void setup() {
  Serial.begin(115200);
  #warning "Serial is enabled"

  // This has to be done first for some reason
  setupSpectrumAnalyzer();

  //analogReference(AR_DEFAULT); // Not on ESP32?
  pinMode(LED_BUILTIN, OUTPUT);

  //FastLED.addLeds<WS2812B, LED_PINS[0], GRB>(leds[0], LEDS_PER_STRIP);
  //FastLED.addLeds<WS2812B, LED_PINS[1], GRB>(leds[1], LEDS_PER_STRIP);
  //FastLED.addLeds<WS2812B, LED_PINS[2], GRB>(leds[2], LEDS_PER_STRIP);
  //FastLED.addLeds<WS2812B, LED_PINS[3], GRB>(leds[3], LEDS_PER_STRIP);
  //FastLED.addLeds<WS2812B, LED_PINS[4], GRB>(leds[4], LEDS_PER_STRIP);
  //FastLED.setBrightness(RemoteXY.brightnessSlider);
  //FastLED.setMaxPowerInVoltsAndMilliamps(5, 5000);

  //// The boot button is connected to GPIO0
  //pinMode(0, INPUT);
  //attachInterrupt(0, buttonInterrupt, FALLING);

  for (int i = 0; i < 3; ++i) {
    blink(100);
  }
}

void loop() {
  spectrumAnalyzer();
}
