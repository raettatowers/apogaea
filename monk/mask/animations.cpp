/** Simpler animations go here */

#include <Arduino.h>
#include <cstdint>
#include <FastLED.h>

#include "constants.hpp"


extern CRGB leds[LED_COUNT];


void binaryClock(uint8_t hue) {
  void showNumber(uint32_t number, const CHSV & color);
  // Do a binary shift instead of integer division because of speed and code size.
  // It's a little less precise, but who cares.
  // Show 1/4 seconds instead of full seconds because it's more interesting. It
  // updates a lot faster and the other lens lights up faster.
  auto now = millis() >> 8;
  const CHSV color(hue, 0xFF, 0xFF);
  showNumber(now, color);
  delay(100);
}
void showNumber(uint32_t number, const CHSV& color) {
  uint8_t counter = 0;
  while (number > 0) {
    if (number & 1) {
      leds[counter] = color;
    } else {
      leds[counter] = CRGB::Black;
    }
    number >>= 1;
    counter += 2;
  }
  FastLED.show();
}


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


void circularWipe(const uint8_t hue) {
  const int HUE_INCREMENT = 30;
  static uint8_t head = 0;
  static uint8_t hueOffset = 0;

  const CRGB previousColor = CHSV(hue + hueOffset, 0xFF, 0xFF);
  const CRGB currentColor = CHSV(hue + hueOffset + HUE_INCREMENT, 0xFF, 0xFF);

  fill_solid(&leds[0], head, currentColor);
  fill_solid(&leds[head], LED_COUNT - head, previousColor);
  if (head < LED_COUNT) {
    ++head;
  } else {
    head = 0;
    hueOffset += HUE_INCREMENT;
  }
  FastLED.show();
  delay(100);
}


void fadingSparks(uint8_t) {
  // Like random sparks, but they fade in and out
  static bool increasing[LED_COUNT] = {false};
  static int8_t brightnessIndexes[LED_COUNT] = {0};
  // Let's keep them the same color that they started with
  static uint8_t hues[LED_COUNT] = {0};
  // Let's use our own sparkHue so that we can change the pixels more quickly
  static uint8_t sparkHue = 0;
  // Start with some zeroes so that we don't relight a pixel immediately
  const static uint8_t rippleBrightnesses[] = {0, 0, 0, 0, 0, 0, 4, 8, 16, 32, 48, 64, 96, 128, 160, 192, 192, 192, 192};

  // So we pick a random LED to start making brighter
  const uint8_t led = random(LED_COUNT);
  if (!increasing[led]) {
    increasing[led] = true;
    hues[led] = sparkHue;
  }
  sparkHue += 3;

  for (uint8_t i = 0; i < COUNT_OF(increasing); ++i) {
    if (increasing[i]) {
      if (brightnessIndexes[i] < static_cast<int>(COUNT_OF(rippleBrightnesses)) - 1) {
        ++brightnessIndexes[i];
      } else {
        increasing[i] = false;
      }
      leds[i] = CHSV(hues[i], 0xFF, rippleBrightnesses[brightnessIndexes[i]]);
    } else {
      if (brightnessIndexes[i] > 0) {
        --brightnessIndexes[i];
        leds[i] = CHSV(hues[i], 0xFF, rippleBrightnesses[brightnessIndexes[i]]);
      }
    }
  }
  delay(100);
  FastLED.show();
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


void shimmer(const uint8_t hue) {
  // Start above 0 so that each light should be on a bit
  static const uint8_t brightness[] = {4, 8, 16, 32, 64, 96, 128};
  static uint8_t values[LED_COUNT];
  extern bool reset;
  if (reset) {
    reset = false;
    for (auto& value : values) {
      value = COUNT_OF(brightness) / 2;
    }
  }

  for (int i = 0; i < 4; ++i) {
    const uint8_t pixel = random(LED_COUNT);
    if (random(2) == 1) {
      if (values[pixel] < COUNT_OF(brightness) - 1) {
        ++values[pixel];
      } else {
        --values[pixel];
      }
    } else {
      if (values[pixel] > 0) {
        --values[pixel];
      } else {
        ++values[pixel];
      }
    }
  }

  for (int i = 0; i < LED_COUNT; ++i) {
    leds[i] = CHSV(hue, 0xFF, brightness[values[i]]);
  }
  FastLED.show();
  delay(20);
}
