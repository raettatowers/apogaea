#include "animations.hpp"
#include <FastLED.h>

Count::Count(CRGB& leds_, int ledCount_) :
  leds(leds),
  ledCount(ledCount),
  index(0),
  previousMillis(millis())
{
}


void Count::animate() {
  const typeof(millis()) millisPerIteration = 500;

  fill_solid(leds, ledCount, CRGB::BLACK);
  leds[index] = color;

  const auto now = millis();
  if (now > previousMillis + millisPerIteration) {
    previousMillis = now;
    ++index;
    if (index >= COUNT_OF(countLeds)) {
      index = 0;
    }
  }
}


Snake::Snake(CRGB& leds_, int ledCount_) :
  leds(leds),
  ledCount(ledCount),
  index(0),
  length(5)
{
}


void Snake::animate(const CRGB& color) {
  const unsigned millisPerIteration = 100;
  const int length = 5;

  fill_solid(leds, ledCount, CRGB::BLACK);
  for (int i = 0; i < 
  leds[index] = color;

  const auto now = millis();
  if (now > previousMillis + millisPerIteration) {
    previousMillis = now;
    ++index;
    if (index >= COUNT_OF(countLeds)) {
      index = 0;
    }
  }
}


ShowBrightness::ShowBrightness(CRGB& leds_, int ledCount_) :
  leds(leds),
  ledCount(ledCount),
  index(0),
  length(5)
{
}


void ShowBrightness::animate() {
  const int MILLIS_PER_ITERATION = 4000;
  static auto previousMillis = MILLIS_PER_ITERATION;
  static int index = 0;
  const uint8_t brightnesses[] = {255, 128, 64, 32, 16};
  static_assert(COUNT_OF(brightnesses) <= COUNT_OF(brightnessLeds));

  // Turn off all the LEDs
  fill_solid(brightnessLeds, COUNT_OF(countLeds), BLACK);

  for (int i = COUNT_OF(brightnesses); i > COUNT_OF(brightnesses) - index - 1 && i >= 0; --i) {
    brightnessLeds[i] = color;
  }

  const auto now = millis();
  if (now > previousMillis + MILLIS_PER_ITERATION) {
    previousMillis = now;
    ++index;
    if (index >= COUNT_OF(brightnesses)) {
      index = 0;
    }
  }
}
