#include <stdint.h>
#include <FastLED.h>

#include "animations.hpp"
#include "constants.hpp"


extern CRGB leds[LED_COUNT];

void Animation::setLed(int x, int y, const CRGB& color) {
  if (x >= 0 && x < LED_COLUMN_COUNT) {
    if (y >= 0 && y < LED_ROW_COUNT) {
      const int index = LED_STRIPS[x][y];
      if (index != UNUSED_LED) {
        leds[LED_STRIPS[x][y]] = color;
      }
    }
  }
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
  if (index == LED_COUNT) {
    index = 0;
  }
  return millisPerIteration;
}


CountXY::CountXY() : index(0) {
}


int CountXY::animate(uint8_t) {
  const int millisPerIteration = 500;
  resetLeds();

  // Highlight the top and bottom of each column
  for (int x = 0; x < LED_COLUMN_COUNT; ++x) {
    if (LED_STRIPS[x][0] != UNUSED_LED) {
      setLed(x, 0, CRGB::Yellow);
    }

    int y;
    for (y = 0; y < LED_ROW_COUNT; ++y) {
      if (LED_STRIPS[x][y] == UNUSED_LED) {
        break;
      }
    }
    if (y > 0) {
      --y;
    }
    if (LED_STRIPS[x][y] != UNUSED_LED) {
      setLed(x, y, CRGB::Aquamarine);
    }
  }

  index = (index + 1) % LED_COUNT;
  leds[index] = CRGB::Red;

  return millisPerIteration;
}


HorizontalSnake::HorizontalSnake() : x(0), y(0), xIncreasing(true)
{
}


int HorizontalSnake::animate(const uint8_t hue) {
  resetLeds();
  if (xIncreasing) {
    ++x;
    if (x >= LED_COLUMN_COUNT) {
      --x;
      xIncreasing = false;
      ++y;
      if (y >= LED_ROW_COUNT) {
        y = 0;
      }
    }
  } else {
    --x;
    if (x < 0) {
      ++x;
      xIncreasing = true;
      ++y;
      if (y >= LED_ROW_COUNT) {
        y = 0;
      }
    }
  }
  setLed(x, y, CHSV(hue, 255, 255));
  return 250;
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


Shine::Shine() : increasing(), amount() {
  static_assert(COUNT_OF(increasing) == LED_COUNT);
  static_assert(COUNT_OF(increasing) == COUNT_OF(amount));
  static_assert(COUNT_OF(amount) == COUNT_OF(hues));

  for (int i = 0; i < LED_COUNT; ++i) {
    increasing[i] = 0;
    amount[i] = 0;
    hues[i] = 0;
  }
}


int Shine::animate(uint8_t hue) {
  const int millisPerIteration = 100;
  const int maxAmount = 120;
  const int changeAmount = 10;
  static_assert(maxAmount % changeAmount == 0);

  hue *= 4;  // Make it cycle faster
  resetLeds();

  // Randomly start increasing an LED
  int chosen = rand() % LED_COUNT;
  if (amount[chosen] == 0) {
    increasing[chosen] = true;
    amount[chosen] = changeAmount;
    hues[chosen] = hue;
  }

  for (int i = 0; i < LED_COUNT; ++i) {
    if (amount[i] > 0) {
      setLed(i, CHSV(hues[i], 255, amount[i]));
      if (increasing[i]) {
        amount[i] += changeAmount;
        // Do >= just to be defensive. I don't expect it to ever be >.
        if (amount[i] >= maxAmount) {
          increasing[i] = false;
        }
      } else {
        amount[i] -= changeAmount;
      }
    }
  }
  return millisPerIteration;
}


Blobs::Blobs(const int count_) :
  count(count_),
  targetX(new float[count_]),
  targetY(new float[count_]),
  x(new float[count_]),
  y(new float[count_]),
  xSpeed(new float[count_]),
  ySpeed(new float[count_])
{
  for (int i = 0; i < count; ++i) {
    targetX[i] = static_cast<float>(rand() % LED_COLUMN_COUNT);
    targetY[i] = static_cast<float>(rand() % LED_ROW_COUNT);
    x[i] = static_cast<float>(rand() % LED_COLUMN_COUNT);
    y[i] = static_cast<float>(rand() % LED_ROW_COUNT);
    xSpeed[i] = 0.0f;
    ySpeed[i] = 0.0f;
  }
}


int Blobs::animate(const uint8_t hue) {
  const int millisPerIteration = 50;
  const float speedChange = 0.05f;
  const float maxSpeed = 3 * speedChange;
  const float radius = 2.0f;
  const float radius2 = radius * radius;
  const float reachedDistance2 = 2.0f;

  resetLeds();

  for (int i = 0; i < count; ++i) {
    const float distance2 = (targetX[i] - x[i]) * (targetX[i] - x[i]) + (targetY[i] - y[i]) * (targetY[i] - y[i]);
    if (distance2 < reachedDistance2) {
      targetX[i] = static_cast<float>(rand() % LED_COLUMN_COUNT);
      targetY[i] = static_cast<float>(rand() % LED_ROW_COUNT);
    }

    // TODO: Update the speed based on the direction to the target
    if (x[i] < targetX[i]) {
      xSpeed[i] += speedChange;
      if (xSpeed[i] > maxSpeed) {
        xSpeed[i] = maxSpeed;
      }
    } else {
      xSpeed[i] -= speedChange;
      if (xSpeed[i] < -maxSpeed) {
        xSpeed[i] = -maxSpeed;
      }
    }

    if (y[i] < targetY[i]) {
      ySpeed[i] += speedChange;
      if (ySpeed[i] > maxSpeed) {
        ySpeed[i] = maxSpeed;
      }
    } else {
      ySpeed[i] -= speedChange;
      if (ySpeed[i] < -maxSpeed) {
        ySpeed[i] = -maxSpeed;
      }
    }

    x[i] += xSpeed[i];
    y[i] += ySpeed[i];

    // Just find all the LEDs in range
    const int startX = static_cast<int>(x[i] - radius);
    const int endX = static_cast<int>(x[i] + radius + 1);
    const int startY = static_cast<int>(y[i] - radius);
    const int endY = static_cast<int>(y[i] + radius + 1);
    const uint8_t hueOffset = 256 / count * i;
    for (int xIter = startX; xIter <= endX; ++xIter) {
      for (int yIter = startY; yIter <= endY; ++yIter) {
        const float distance2 = static_cast<float>((xIter - x[i]) * (xIter - x[i]) + (yIter - y[i]) * (yIter - y[i]));
        const float ratio = (radius2 - distance2) / radius2;
        if (ratio > 0.0f) {
          const int brightness = static_cast<int>(255 * ratio);
          setLed(xIter, yIter, CHSV(hue + hueOffset, 255, brightness));
        }
      }
    }
  }

  return millisPerIteration;
}


Blobs::~Blobs() {
  delete [] targetX;
  delete [] targetY;
  delete [] x;
  delete [] y;
  delete [] xSpeed;
  delete [] ySpeed;
}


Plasma::Plasma(float multiplier_, float timeIncrement_) :
  time(0),
  multiplier(multiplier_),
  timeIncrement(timeIncrement_)
{
}


int Plasma::animate(uint8_t) {
  // Adapted from https://www.bidouille.org/prog/plasma
  const float M_PI_F = static_cast<float>(M_PI);

  resetLeds();
  time += timeIncrement;
  float cys[LED_ROW_COUNT];
  for (int y = 0; y < LED_ROW_COUNT; ++y) {
    cys[y] = y + 0.5f * cosf(time * (1.0f / 3.0f));
  }
  for (int x = 0; x < LED_COLUMN_COUNT; ++x) {
    const float v1 = sinf(multiplier * x + time);
    const float cx = sinf(x + 0.5 * sinf(time * 0.2f));
    for (int y = 0; y < LED_ROW_COUNT; ++y) {
      if (LED_STRIPS[x][y] != UNUSED_LED) {
        const float v2 = sinf(multiplier * (x * sinf(time * 0.5f) + y * cosf(time * (1.0f / 3.0f))) + time);
        const float v3 = sinf(multiplier * sqrtf((cx * cx + cys[y] * cys[y] + 1.0f)) + time);
        const float v = v1 + v2 + v3;
        //const float v = v1 + v2;
        const float red = sinf(v * M_PI_F);
        const float green = sinf(v * M_PI_F + (2.0f / 3.0f) * M_PI_F);
        const float blue = sinf(v * M_PI_F + (4.0f / 3.0f) * M_PI_F);
        const uint8_t r = convert(red);
        const uint8_t g = convert(green);
        const uint8_t b = convert(blue);
        setLed(x, y, CRGB(r, g, b));
      }
    }
  }
  return 0;
}
