#include <FastLED.h>

const int LED_PIN = 7;
const int LED_COUNT = 45;


CRGB leds[LED_COUNT];

void setup() { 
  FastLED.addLeds<WS2812B, LED_PIN>(leds, LED_COUNT);
  FastLED.setBrightness(.1 * 255);
}


void loop() {
  uint8_t rippleStart = 0;
  const uint8_t rippleLength = 4;
  const uint8_t brightness[rippleLength] = {10, 30, 60, 120};
  uint8_t hue = 0;
  while (true) {
    FastLED.clear();
    for (uint8_t i = 0; i < rippleLength; ++i) {
      const int index = (rippleStart + i) % LED_COUNT;
      const int rippleHue = hue + i;
      leds[index].setHSV(rippleHue, 0xFF, brightness[i]);
    }
    for (uint8_t i = 0; i < rippleLength; ++i) {
      const int index = (rippleStart + rippleLength + i) % LED_COUNT;
      const int rippleHue = hue + i + rippleLength;
      const int rippleBrightness = brightness[rippleLength - 1 - i];
      leds[index].setHSV(rippleHue, 0xFF, rippleBrightness);
    }
    FastLED.show();
    delay(100);
    ++hue;
    rippleStart = (rippleStart + 1) % LED_COUNT;
  }
}
