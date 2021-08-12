#include <algorithm>
#include <cstdint>

#include "animations.hpp"
#include "constants.hpp"

using std::max;
using std::min;

#ifdef DEMO
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
extern SDL_Renderer* renderer;
namespace demoConstants {
  const int minXPixel = static_cast<int>(640 * 0.15) - 5;
  const int maxXPixel = static_cast<int>(640 * 0.85) - 5;
  const int minYPixel = static_cast<int>(480 * 0.3) + 5;
  const int maxYPixel = static_cast<int>(480 * 0.7) - 5;
  const float xMultiplier = static_cast<float>(maxXPixel - minXPixel) / LED_COLUMN_COUNT;
  const float yMultiplier = static_cast<float>(maxYPixel - minYPixel) / LED_ROW_COUNT;
  const int radius = 3;
}

void Animation::setLed(int x, int y, uint32_t color) {
  using namespace demoConstants;

  const int adjustedX = minXPixel + static_cast<int>(x * xMultiplier);
  const int adjustedY = maxYPixel - static_cast<int>(y * yMultiplier);
  if (adjustedX >= minXPixel && adjustedX <= maxXPixel && adjustedY >= minYPixel && adjustedY <= maxYPixel) {
    if (filledCircleColor(renderer, adjustedX, adjustedY, radius, color) != 0) {
      fprintf(stderr, "filledCircleColor failed\n");
    }
  }
}

void Animation::setLed(int index, uint32_t color) {
  for (int x = 0; x < LED_COLUMN_COUNT; ++x) {
    for (int y = 0; y < LED_ROW_COUNT; ++y) {
      if (LED_STRIPS[x][y] == index) {
        setLed(x, y, color);
        return;
      }
    }
  }
}

void resetLeds() {
  // TODO: FastLED won't wipe changes from previous runs, but I'm clearing SDL
  // on every frame. This should emulate that behavior. Right now it doesn't
  // matter because every animation calls this at the beginning to clear the
  // LEDs anyway.
}

#else
#include <FastLED.h>
extern CRGB leds[LED_COUNT];

static void Animation::setLed(int x, int y, uint32_t color) {
  leds[LED_STRIPS[x][y]] = color;
}
static void Animation::setLed(int index, uint32_t color) {
  leds[index] = color;
}

void resetLeds() {
  fill_solid(leds, ledCount, CRGB::BLACK);
}
#endif


Count::Count() : index(0)
{
}


int Count::animate(const uint32_t color) {
  const int millisPerIteration = 500;

  resetLeds();
  setLed(index, color);
  ++index;
  return millisPerIteration;
}


Snake::Snake() : index(0)
{
}


int Snake::animate(const uint32_t color) {
  const unsigned millisPerIteration = 100;
  const int length = 5;

  resetLeds();
  const int end = min(index, LED_COUNT);
  for (int i = max(index - length, 0); i < end; ++i) {
    setLed(i, color);
  }
  ++index;
  if (index > LED_COUNT) {
    index = 0;
  }

  return millisPerIteration;
}


int ShowBrightness::animate(const uint32_t color) {
  const int millisPerIteration = 4000;
  const uint8_t brightnesses[] = {255, 128, 64, 32, 16};

  resetLeds();

  for (int i = COUNT_OF(brightnesses); i > COUNT_OF(brightnesses) - index - 1 && i >= 0; --i) {
    setLed(i, color);
  }

  return millisPerIteration;
}


Ripple::Ripple() : index(0)
{
}


int Ripple::animate(const uint32_t color) {
  const int millisPerIteration = 250;
  resetLeds();
  ++index;
  if (index >= LED_COLUMN_COUNT + LED_ROW_COUNT) {
    index = 0;
  }
  for (int i = 0; i < index; ++i) {
    setLed(index - i, i, color);
  }

  return millisPerIteration;
}
