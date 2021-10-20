#include <FastLED.h>
#include <SoftwareSerial.h>

#include "capacitive_touch.hpp"

const int LED_COUNT = 10;

const int NEOPIXEL_PIN = 8;
const int LEFT_BUTTON_PIN = 4;
const int RIGHT_BUTTON_PIN = 5;
const int LEFT_PAD_PIN = 0;
const int RIGHT_PAD_PIN = 6;
const int SWITCH_PIN = 7;

CRGB leds[LED_COUNT];

void setup() {
  Serial.begin(115200);
  LEDS.addLeds<WS2812B, NEOPIXEL_PIN, GRB>(leds, LED_COUNT);
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
  static int index = 0;
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
  fill_solid(leds, LED_COUNT, CRGB(red, green ,blue));
  FastLED.show();

  if (needDelay) {
    delay(250);
  }
}
