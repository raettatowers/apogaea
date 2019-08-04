#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <cstdint>

#include "constants.hpp"

using std::uint8_t;

static uint8_t brightnesses[2 * PIXEL_RING_COUNT] = {0};
static int8_t directions[COUNT_OF(brightnesses)] = {0};
static uint8_t delays[COUNT_OF(brightnesses)] = {0};
static uint8_t maxbrightnesses[COUNT_OF(brightnesses)] = {0};
static const int DROP_OFF = 5;
static const int MAX_DROP_OFF = 15;
static const int INCREMENT = 20;
static const int MAX_BRIGHTNESS = 100;
static const int INITIAL_DELAY = 3;


static void ripple(const int pixel) {
  // Each pixel operates independently without looking at its neighbors,
  // increasing up to some max brightness then turning around and decreasing to
  // 0 and then increasing again. Each time max is reached, the max is
  // decreased.  To implement a ripple effect, a delay is introduced for each
  // pixel the further it is from the initial drop before it starts running the
  // cycle.
}

void ripples(Adafruit_NeoPixel& pixels) {
  static_assert(MAX_BRIGHTNESS > COUNT_OF(brightnesses) / 2 * DROP_OFF, "");

  // Each lens runs independently. I tried to get the ripple to carry across
  // but it never looked good.
  for (int lens = 0; lens < 2; ++lens) {
    bool allOff = true;
    for (auto b : brightnesses) {
      if (b > 0) {
        allOff = false;
        break;
      }
    }
    if (allOff) {
      for (auto &d : directions) {
        d = 1;
      }
      // Start next drop
      const uint8_t drop = random(COUNT_OF(brightnesses));
      delays[drop] = 0;
      maxbrightnesses[drop] = MAX_BRIGHTNESS;
      // Prepare the other drops
      for (uint8_t i = 1; i < COUNT_OF(brightnesses) / 2 + 1; ++i) {
        const int index1 = (COUNT_OF(brightnesses) * 2 + drop - i) % COUNT_OF(brightnesses);
        const int index2 = (COUNT_OF(brightnesses) + drop + i) % COUNT_OF(brightnesses);
        delays[index1] = i * INITIAL_DELAY;
        delays[index2] = i * INITIAL_DELAY;
        maxbrightnesses[index1] = MAX_BRIGHTNESS - i * DROP_OFF;
        maxbrightnesses[index2] = MAX_BRIGHTNESS - i * DROP_OFF;
      }
    }

    // Update the drops
    for (uint8_t i = 0; i < COUNT_OF(brightnesses); ++i) {
      if (delays[i] == 0) {
        // Increase toward the max
        if (directions[i] > 0) {
          if (brightnesses[i] + INCREMENT <= maxbrightnesses[i]) {
            brightnesses[i] += INCREMENT;
          } else {
            // Decrease max brightness if we can
            if (maxbrightnesses[i] - MAX_DROP_OFF > 0) {
              brightnesses[i] = maxbrightnesses[i];
              maxbrightnesses[i] -= MAX_DROP_OFF;
              directions[i] = -1;
            } else {
              // All done
              maxbrightnesses[i] = 0;
              brightnesses[i] = 0;
              directions[i] = 0;
            }
          }
          // Decrease toward zero
        } else if (directions[i] < 0) {
          if (brightnesses[i] < INCREMENT) {
            // Turn around
            directions[i] = 1;
            brightnesses[i] = 0;
            // Also add a delay to get a better ripple effect
            delays[i] = 3;
          } else {
            brightnesses[i] -= INCREMENT;
          }
        }
      } else {
        --delays[i];
      }

      uint16_t hue = 0;  // TODO: Change this
      pixels.setPixelColor(i, pixels.ColorHSV(hue, 0xFF, brightnesses[i]));
    }
  }

  pixels.show();
  delay(100);
}
