#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <cstdint>

#include "constants.hpp"

using std::uint8_t;

static uint8_t brightnesses[2][PIXEL_RING_COUNT] = {0};
static int8_t directions[2][PIXEL_RING_COUNT] = {0};
static uint8_t delays[2][PIXEL_RING_COUNT] = {0};
static uint8_t maxBrightnesses[2][PIXEL_RING_COUNT] = {0};
static uint8_t lensDelay[2] = {0};
static const int DROP_OFF = 5;
static const int MAX_DROP_OFF = 15;
static const int INCREMENT = 20;
static const int MAX_BRIGHTNESS = 100;
static const int INITIAL_DELAY = 3;
static const int RIPPLE_DELAY = 3;
static const int LENS_MAX_RANDOM_DELAY = 5;

static void ripple(int lens, int i, Adafruit_NeoPixel* pixels, uint16_t hue);
static void resetLens(int lens);


void ripples(Adafruit_NeoPixel* pixels, uint16_t hue) {
  static_assert(MAX_BRIGHTNESS > COUNT_OF(brightnesses) / 2 * DROP_OFF, "");

  // Each lens runs independently. I tried to get the ripple to carry across
  // but it never looked good.
  for (int lens = 0; lens < 2; ++lens) {
    bool allOff = true;
    for (auto b : brightnesses[lens]) {
      if (b > 0) {
        allOff = false;
        break;
      }
    }
    if (allOff) {
      if (lensDelay[lens] == 0) {
        resetLens(lens);
        lensDelay[lens] = random(LENS_MAX_RANDOM_DELAY);
      } else {
        --lensDelay[lens];
      }
    }

    // Update the drops
    for (uint8_t i = 0; i < PIXEL_RING_COUNT; ++i) {
      ripple(lens, i, pixels, hue);
    }
  }

  pixels->show();
  delay(100);
}


void ripple(const int lens, const int i, Adafruit_NeoPixel* pixels, const uint16_t hue) {
  // Each pixel operates independently without looking at its neighbors,
  // increasing up to some max brightness then turning around and decreasing to
  // 0 and then increasing again. Each time max is reached, the max is
  // decreased. To implement a ripple effect, a delay is introduced for each
  // pixel the further it is from the initial drop before it starts running the
  // cycle.
  if (delays[lens][i] == 0) {
    // Increase toward the max
    if (directions[lens][i] > 0) {
      if (brightnesses[lens][i] + INCREMENT <= maxBrightnesses[lens][i]) {
        brightnesses[lens][i] += INCREMENT;
      } else {
        // Decrease max brightness if we can
        if (maxBrightnesses[lens][i] - MAX_DROP_OFF > 0) {
          brightnesses[lens][i] = maxBrightnesses[lens][i];
          maxBrightnesses[lens][i] -= MAX_DROP_OFF;
          directions[lens][i] = -1;
        } else {
          // All done
          maxBrightnesses[lens][i] = 0;
          brightnesses[lens][i] = 0;
          directions[lens][i] = 0;
        }
      }
      // Decrease toward zero
    } else if (directions[lens][i] < 0) {
      if (brightnesses[lens][i] < INCREMENT) {
        // Turn around
        directions[lens][i] = 1;
        brightnesses[lens][i] = 0;
        // Also add a delay to get a better ripple effect
        delays[lens][i] = RIPPLE_DELAY;
      } else {
        brightnesses[lens][i] -= INCREMENT;
      }
    }
  } else {
    --delays[lens][i];
  }

  pixels->setPixelColor(lens * PIXEL_RING_COUNT + i, pixels->ColorHSV(hue, 0xFF, brightnesses[lens][i]));
}


// lens is either 0 or 1
void resetLens(const int lens) {
  for (auto &d : directions[lens]) {
    d = 1;
  }
  // Start next drop
  const uint8_t drop = random(COUNT_OF(brightnesses[lens]));
  delays[lens][drop] = 0;
  maxBrightnesses[lens][drop] = MAX_BRIGHTNESS;
  brightnesses[lens][drop] = 1;
  // Prepare the other drops
  for (uint8_t i = 1; i < COUNT_OF(brightnesses[lens]) / 2 + 1; ++i) {
    const int index1 = (COUNT_OF(brightnesses[lens]) * 2 + drop - i) % COUNT_OF(brightnesses[lens]);
    const int index2 = (COUNT_OF(brightnesses[lens]) + drop + i) % COUNT_OF(brightnesses[lens]);
    static_assert((COUNT_OF(brightnesses[lens]) / 2 + 1) * INITIAL_DELAY < 0xFF, "");
    delays[lens][index1] = i * INITIAL_DELAY;
    delays[lens][index2] = i * INITIAL_DELAY;
    maxBrightnesses[lens][index1] = MAX_BRIGHTNESS - i * DROP_OFF;
    maxBrightnesses[lens][index2] = MAX_BRIGHTNESS - i * DROP_OFF;
  }
}
