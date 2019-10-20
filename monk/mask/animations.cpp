/** Simpler animations go here */

#include <Arduino.h>
#include <cstdint>
#include <FastLED.h>

#include "constants.hpp"


extern CRGB leds[LED_COUNT];

void breathe(const uint8_t hue) {
  static uint8_t brightness = 1;
  static decltype(millis()) zeroTimeout_ms = 0;
  static uint8_t step = 1;

  const decltype(millis()) zeroPauseTime_ms = 1000;

  extern bool reset;
  if (reset) {
    reset = false;
    brightness = 1;
    zeroTimeout_ms = 0;
    step = 1;
  }

  FastLED.clear();
  if (zeroTimeout_ms > 0) {
    if (millis() > zeroTimeout_ms) {
      zeroTimeout_ms = 0;
    }
  } else {
    brightness += step;
    if (brightness == 255) {
      step = -1;
    } else if (brightness == 0) {
      step = 1;
      zeroTimeout_ms = millis() + zeroPauseTime_ms;
    }
    for (auto& value : leds) {
      value.setHSV(hue, 0xFF, brightness);
    }
  }
  FastLED.show();
  delay(10);
}


void rainbowSwirl(const uint8_t hue) {
  const uint8_t brightness[] = {30, 60, 90, 120, 180, 240};
  static uint8_t swirlStart = 0;

  FastLED.clear();
  for (uint8_t i = 0; i < COUNT_OF(brightness); ++i) {
    const int index = (swirlStart + i) % LED_COUNT;
    const int swirlHue = hue + i;
    leds[index].setHSV(swirlHue, 0xFF, brightness[i]);
  }
  FastLED.show();
  delay(40);
  swirlStart = (swirlStart + 1) % LED_COUNT;
}
