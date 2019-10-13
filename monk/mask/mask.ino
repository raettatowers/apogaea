#include <FastLED.h>

#include "constants.hpp"

const int LED_PIN = 7;
CRGB leds[LED_COUNT];


void setup() {
  FastLED.addLeds<WS2812B, LED_PIN>(leds, LED_COUNT);
  FastLED.setBrightness(1.0 * 255);
  Serial.begin(9600);
}


void loop() {
  oneAtATime();
}


void oneAtATime() {
  static uint8_t led = 9;

  if (Serial.available() > 0) {
    char arr[5];
    Serial.readBytesUntil('\n', arr, COUNT_OF(arr) - 1);
    while (Serial.available() > 0) {
      Serial.read();
    }
    arr[COUNT_OF(arr) - 1] = '\0';
    led = min(atoi(arr), LED_COUNT);
  }

  FastLED.clear();
  leds[led] = CRGB::White;
  FastLED.show();
  delay(10);
}


void oneAtATimeCycle() {
  static uint8_t led = 0;
  static bool rainbow = true;
  static uint32_t colorCodes[] = {0x0000FF, 0x00FF00, 0x00FFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF};
  static uint8_t colorIndex = 0;
  static typeof(millis()) colorTime_ms;
  static uint8_t hue = 0;

  const typeof(millis()) colorTimeout_ms = 3 * 1000;

  if (Serial.available() > 0) {
    char arr[5];
    Serial.readBytesUntil('\n', arr, COUNT_OF(arr) - 1);
    while (Serial.available() > 0) {
      Serial.read();
    }
    arr[COUNT_OF(arr) - 1] = '\0';
    led = min(atoi(arr), LED_COUNT);
  }

  FastLED.clear();
  if (rainbow) {
    leds[led] = CHSV(hue, 0xFF, 0xFF);

    ++hue;
    if (hue == 0) {
      rainbow = false;
      colorTime_ms = millis();
    }
  } else {
    leds[led] = CRGB(colorCodes[colorIndex]);

    if (millis() - colorTime_ms > colorTimeout_ms) {
      colorTime_ms = millis();
      ++colorIndex;
      if (colorIndex >= COUNT_OF(colorCodes)) {
        colorIndex = 0;
        rainbow = true;
      }
    }
  }
  FastLED.show();
  delay(10);
}


void rainbowRipple() {
  const uint8_t rippleLength = 4;
  const uint8_t brightness[rippleLength] = {30, 60, 120, 240};
  static uint8_t rippleStart = 0;
  static uint8_t hue = 0;

  FastLED.clear();
  for (uint8_t i = 0; i < rippleLength; ++i) {
    const int index = (rippleStart + i) % LED_COUNT;
    const int rippleHue = hue + i;
    leds[index].setHSV(rippleHue, 0xFF, brightness[i]);
  }
  FastLED.show();
  delay(40);
  ++hue;
  rippleStart = (rippleStart + 1) % LED_COUNT;
}

