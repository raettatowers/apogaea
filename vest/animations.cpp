#include <algorithm>
#include <cstdint>
#include <FastLED.h>

#include "animations.hpp"
#include "constants.hpp"

using std::max;
using std::min;

extern CRGB leds[LED_COUNT];

void Animation::setLed(int x, int y, const CRGB& color) {
  leds[LED_STRIPS[x][y]] = color;
}
void Animation::setLed(int index, const CRGB& color) {
  leds[index] = color;
}

void resetLeds() {
  fill_solid(leds, LED_COUNT, CRGB::Black);
}


Count::Count() : index(0)
{
}


int Count::animate(const uint8_t hue) {
  const int millisPerIteration = 500;

  resetLeds();
  leds[index] = CHSV(hue, 255, 255);
  ++index;
  return millisPerIteration;
}


Snake::Snake() : startIndex(0), endIndex(0)
{
}


int Snake::animate(const uint8_t hue) {
  const unsigned millisPerIteration = 20;
  const int length = 5;

  resetLeds();

  // Snake just entering
  if (endIndex < length) {
    fill_rainbow(&leds[0], endIndex, hue);
    ++endIndex;
    return millisPerIteration;
  } else {
    // Snake in the middle or exiting
    const int endLength = min(length, LED_COUNT - startIndex);
    fill_rainbow(&leds[startIndex], endLength, hue);

    ++startIndex;
    if (startIndex >= COUNT_OF(leds)) {
      startIndex = 0;
      endIndex = 1;
    }
  }
  return millisPerIteration;
}


ShowBrightness::ShowBrightness() : index(10) {
}


int ShowBrightness::animate(const uint8_t hue) {
  const int millisPerIteration = 4000;
  const uint8_t brightnesses[] = {255, 128, 64, 32, 16};

  resetLeds();

  for (int i = COUNT_OF(brightnesses); i > COUNT_OF(brightnesses) - index - 1 && i >= 0; --i) {
    setLed(i, CHSV(hue, 255, 255));
  }

  return millisPerIteration;
}


Ripple::Ripple() : index(0)
{
}


int Ripple::animate(const uint8_t hue) {
  const int millisPerIteration = 250;
  resetLeds();
  ++index;
  const CRGB color = CHSV(hue, 255, 255);
  if (index >= LED_COLUMN_COUNT + LED_ROW_COUNT) {
    index = 0;
  }
  for (int i = 0; i < index; ++i) {
    setLed(index - i, i, color);
  }

  return millisPerIteration;
}
