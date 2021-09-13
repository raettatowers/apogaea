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


Snake::Snake(int length_, int count_) :
  length(length_),
  count(count_),
  startIndexes(new int[count_]),
  endIndexes(new int[count_])
{
  for (int i = 0; i < count; ++i) {
    startIndexes[i] = static_cast<int>(static_cast<float>(LED_COUNT) / count * i);
    // Force all the snakes to start entirely on the vest
    endIndexes[i] = LED_COUNT;
  }
}


int Snake::animate(const uint8_t originalHue) {
  const unsigned millisPerIteration = 20;

  resetLeds();

  for (int i = 0; i < count; ++i) {
    // Make the hues cycle faster
    const uint8_t hue = originalHue * 4 + (255 / count) * i;

    // Snake just entering
    if (endIndexes[i] < length) {
      fill_rainbow(&leds[0], endIndexes[i], hue);
      ++endIndexes[i];
    } else {
      // Snake in the middle or exiting
      const int endLength = min(length, LED_COUNT - startIndexes[i]);
      fill_rainbow(&leds[startIndexes[i]], endLength, hue);

      ++startIndexes[i];
      if (startIndexes[i] >= COUNT_OF(leds)) {
        startIndexes[i] = 0;
        endIndexes[i] = 1;
      }
    }
  }
  return millisPerIteration;
}


Snake::~Snake() {
  delete [] startIndexes;
  delete [] endIndexes;
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
    const int startX = max(static_cast<int>(x[i] - radius), 0);
    const int endX = min(static_cast<int>(x[i] + radius + 1), LED_COLUMN_COUNT);
    const int startY = max(static_cast<int>(y[i] - radius), 0);
    const int endY = min(static_cast<int>(y[i] + radius + 1), LED_ROW_COUNT);
    const uint8_t hueOffset = 256 / count * i;
    for (int xIter = startX; xIter < endX; ++xIter) {
      for (int yIter = startY; yIter < endY; ++yIter) {
        const auto index = LED_STRIPS[xIter][yIter];
        if (index == UNUSED_LED) {
          continue;
        }
        const float distance2 = static_cast<float>((xIter - x[i]) * (xIter - x[i]) + (yIter - y[i]) * (yIter - y[i]));
        const float ratio = sqrtf((radius2 - distance2) / radius2);
        if (ratio > 0.0f) {
          const int brightness = static_cast<int>(255 * ratio);
          const CHSV color = CHSV(hue + hueOffset, 255, brightness);
          if (leds[index]) {
            const CRGB blended = blend(color, leds[index], 128);
            setLed(xIter, yIter, blended);
          } else {
            setLed(xIter, yIter, color);
          }
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


PlasmaBidoulle::PlasmaBidoulle(float multiplier_, float timeIncrement_) :
  time(0),
  multiplier(multiplier_),
  timeIncrement(timeIncrement_),
  redMultiplier(1.0f),
  greenMultiplier(1.0f),
  blueMultiplier(1.0f),
  redOffset(0.0f),
  greenOffset(2.0f / 3.0f * M_PI),
  blueOffset(4.0f / 3.0f * M_PI)
{
}


PlasmaBidoulle::PlasmaBidoulle(
  float multiplier_,
  float timeIncrement_,
  float redMultiplier_,
  float greenMultiplier_,
  float blueMultiplier_,
  float redOffset_,
  float greenOffset_,
  float blueOffset_
) : time(0),
  multiplier(multiplier_),
  timeIncrement(timeIncrement_),
  redMultiplier(redMultiplier_),
  greenMultiplier(greenMultiplier_),
  blueMultiplier(blueMultiplier_),
  redOffset(redOffset_),
  greenOffset(greenOffset_),
  blueOffset(blueOffset_)
{
}


uint8_t PlasmaBidoulle::convert(const float f) {
  return static_cast<uint8_t>((f + 1.0f) * 0.5f * 255.0f);
}


int PlasmaBidoulle::animate(uint8_t) {
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
        const float red = redMultiplier * sinf(v * M_PI_F + redOffset);
        const float green = greenMultiplier * sinf(v * M_PI_F + greenOffset);
        const float blue = blueMultiplier * sinf(v * M_PI_F + blueOffset);
        const uint8_t r = convert(red);
        const uint8_t g = convert(green);
        const uint8_t b = convert(blue);
        setLed(x, y, CRGB(r, g, b));
      }
    }
  }
  return 0;
}


PlasmaBidoulleFast::PlasmaBidoulleFast(ColorGenerator& colorGenerator_) :
  colorGenerator(colorGenerator_),
  time(0)
{
}


int PlasmaBidoulleFast::animate(uint8_t) {
  // Partially adapted from https://www.bidouille.org/prog/plasma, using fast integer
  // math. This fast implementation isn't perfect. I couldn't get v3 working right.
  // But it still looks decent.
  const uint16_t multiplier = 0.15f * PI_16_1_0;
  const uint16_t timeIncrement = 0.1f * PI_16_1_0;

  time += timeIncrement;
  /*
  int32_t cys[LED_ROW_COUNT];
  for (int y = 0; y < LED_ROW_COUNT; ++y) {
    cys[y] = y * 32768 + cos16(time / 3) / 2;  // good
  }
  */
  for (int x = 0; x < LED_COLUMN_COUNT; ++x) {
    const int16_t v1 = sin16(multiplier * x + time); // good
    // const int16_t cx = sin16(multiplier * x + sin16(time / 5) / 2);  // bad
    for (int y = 0; y < LED_ROW_COUNT; ++y) {
      if (LED_STRIPS[x][y] != UNUSED_LED) {
        const int16_t v2 = sin16(
            (multiplier * (x * sin16(time / 4) / 2 + y * cos16(time / 3)) + time) /
            16384); // bad values, but looks good?
        /*
        const int16_t v3 = sin16(sqrtf((cx * cx + cys[y] * cys[y] + 1)) *
        PI_16_1_0 + time);  // bad
        */
        // const int16_t v = v1 + v2 + v3;
        const int8_t v = (v1 + v2) / 256 + 128;
        setLed(x, y, colorGenerator.getColor(v));
      }
    }
  }
  return 20;
}


Plasma1::Plasma1(ColorGenerator& colorGenerator_) :
  colorGenerator(colorGenerator_),
  time(0)
{
}


int Plasma1::animate(uint8_t) {
  for (int x = 0; x < LED_COLUMN_COUNT; ++x) {
    for (int y = 0; y < LED_ROW_COUNT; ++y) {
      const auto index = LED_STRIPS[x][y];
      if (index != UNUSED_LED) {
        const uint8_t p1 = 128 + (sin16(x * (PI_16_1_0 / 4) + time / 4)) / 256;
        const uint8_t p2 = 128 + (sin16(y * (PI_16_1_0 / 4) + time / 4)) / 256;
        const uint8_t p3 =
          128 + (sin16((x + y) * (PI_16_1_0 / 8) + time / 8)) / 256;
        const uint8_t p4 =
          128 + (sin16(sqrt16(x * x + y * y) * 1000 + time)) / 256;
        const uint8_t v = (p1 + p2 + p3 + p4) / 4;
        setLed(x, y, colorGenerator.getColor(v));
      }
    }
  }
  time += 1000;
  return 10;
}

Plasma2::Plasma2(ColorGenerator& colorGenerator_) :
  colorGenerator(colorGenerator_),
  time(0)
{
}


int Plasma2::animate(uint8_t) {
  for (int x = 0; x < LED_COLUMN_COUNT; ++x) {
    for (int y = 0; y < LED_ROW_COUNT; ++y) {
      const auto index = LED_STRIPS[x][y];
      if (index != UNUSED_LED) {
        const uint8_t p1 = 128 + (sin16(x * (PI_16_1_0 / 8) + time / 4)) / 256;
        const uint8_t p2 =
          128 +
          sin16(10 * (x * sin16(time / 2) / 256 + y * cos16(time / 3) / 256)) /
          256;
        const uint8_t p3 =
          128 + (sin16((x + y) * (PI_16_1_0 / 8) + time / 8)) / 256;
        // cx = x + 0.5 * sin(time / 5)
        // cy = y + 0.5 * cos(time / 3)
        // v = sin(sqrt(100 * (cx**2 + cy ** 2) + 1) + time)
        const uint16_t cx = x + sin16(time / 8) / 1024;
        const uint16_t cy = y + sin16(time / 16) / 1024;
        const uint8_t p4 =
          128 + sin16(sqrt16(cx * cx + cy * cy) * (PI_16_1_0 / 2) + time) / 512;
        const uint8_t v = (p1 + p2 + p3 + p4) / 4;
        setLed(x, y, colorGenerator.getColor(v));
      }
    }
  }
  time += 1000;
  return 10;
}

Plasma3::Plasma3(ColorGenerator& colorGenerator_) :
  colorGenerator(colorGenerator_),
  time(0)
{
}


int Plasma3::animate(uint8_t) {
  for (int x = 0; x < LED_COLUMN_COUNT; ++x) {
    for (int y = 0; y < LED_ROW_COUNT; ++y) {
      const auto index = LED_STRIPS[x][y];
      if (index != UNUSED_LED) {
        const uint8_t p1 = 128 + (sin16(x * (PI_16_1_0 / 8) + time / 4)) / 256;
        const uint8_t p2 =
          128 +
          sin16(10 * (x * sin16(time / 2) / 256 + y * cos16(time / 3) / 256)) /
          256;
        const uint8_t p3 =
          128 + (sin16((x + y) * (PI_16_1_0 / 8) + time / 8)) / 256;
        // cx = x + 0.5 * sin(time / 5)
        // cy = y + 0.5 * cos(time / 3)
        // v = sin(sqrt(100 * (cx**2 + cy ** 2) + 1) + time)
        const uint16_t cx = x + sin16(time / 8) / 1024;
        const uint16_t cy = y + sin16(time / 16) / 1024;
        const uint8_t p4 =
          128 + sin16(sqrt16(cx * cx + cy * cy) * (PI_16_1_0 / 2) + time) / 512;
        const uint8_t v = (p1 + p2 + p3 + p4) / 4;
        setLed(x, y, colorGenerator.getColor(v));
      }
    }
  }
  time += 1000;
  return 10;
}
