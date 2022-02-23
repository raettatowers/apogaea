#include <FastLED.h>
#include <SoftwareSerial.h>

#include "capacitive_touch.hpp"

const int INTERNAL_LED_COUNT = 1;
const int EXTERNAL_LED_COUNT = 5;

const int NEOPIXEL_PIN = 8;
const int EXTERNAL_PIN = 16; // 16 = A2 / D9
const int DATA_PIN = 16; // 16 = A2 / D9
const int CLOCK_PIN = 15; // 15 = A1 / D6
const int LEFT_BUTTON_PIN = 4;
const int RIGHT_BUTTON_PIN = 5;
const int LEFT_PAD_PIN = 0;
const int RIGHT_PAD_PIN = 6;
const int SWITCH_PIN = 7;

CRGB internalLeds[INTERNAL_LED_COUNT];
CRGB externalLeds[EXTERNAL_LED_COUNT];

void setup() {
  Serial.begin(115200);
  LEDS.addLeds<WS2812B, NEOPIXEL_PIN, GRB>(internalLeds, INTERNAL_LED_COUNT);
  LEDS.addLeds<APA102, DATA_PIN, CLOCK_PIN, RGB>(externalLeds, EXTERNAL_LED_COUNT);
  LEDS.setBrightness(255);
  pinMode(LEFT_BUTTON_PIN, INPUT_PULLDOWN);
  pinMode(RIGHT_BUTTON_PIN, INPUT_PULLDOWN);
  pinMode(SWITCH_PIN, INPUT);
  pinMode(6, INPUT);
}


void loop() {
  static uint8_t red;
  static uint8_t green;
  static uint8_t blue;
  static TouchIn leftPad(LEFT_PAD_PIN);
  static TouchIn rightPad(RIGHT_PAD_PIN);

  bool needDelay = false;

  if (leftPad.touched()) {
    ++red;
    if (red == 0) {
      needDelay = true;
    }
  }
  if (digitalRead(LEFT_BUTTON_PIN) == HIGH) {
    ++green;
    if (green == 0) {
      needDelay = true;
    }
  }
  if (digitalRead(RIGHT_BUTTON_PIN) == HIGH) {
    ++blue;
    if (blue == 0) {
      needDelay = true;
    }
  }
  if (rightPad.touched()) {
    red = green = blue = 0;
  }

  FastLED.clear();
  fill_solid(internalLeds, INTERNAL_LED_COUNT, CRGB(red, green ,blue));
  fill_solid(externalLeds, EXTERNAL_LED_COUNT, CRGB(red, green ,blue));
  FastLED.show();

  if (needDelay) {
    delay(250);
  }
}
