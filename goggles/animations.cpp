/** Simpler animations go here */

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <cstdint>

#include "constants.hpp"

static void showNumber(Adafruit_NeoPixel* pixels, uint32_t number, const uint32_t color);
static void copyLens(Adafruit_NeoPixel* pixels, uint8_t rotate = 0);
static void mirrorLens(Adafruit_NeoPixel* pixels, uint8_t rotate = 0);

void binaryClock(Adafruit_NeoPixel* pixels, uint16_t hue) {
  // Do a binary shift instead of integer division because of speed and code size.
  // It's a little less precise, but who cares.
  // Show 1/4 seconds instead of full seconds because it's more interesting. It
  // updates a lot faster and the other lens lights up faster.
  uint32_t now = millis() >> 8;
  const uint32_t color = pixels->ColorHSV(hue);
  showNumber(pixels, now, color);
  delay(100);
}


static void showNumber(Adafruit_NeoPixel* pixels, uint32_t number, const uint32_t color) {
  uint8_t counter = 0;
  while (number > 0) {
    if (number & 1) {
      pixels->setPixelColor(counter, color);
    } else {
      pixels->setPixelColor(counter, 0);
    }
    number >>= 1;
    counter += 2;
  }
  pixels->show();
}


void fadingSparks(Adafruit_NeoPixel* pixels, uint16_t) {
  // Like random sparks, but they fade in and out
  static bool increasing[2 * PIXEL_RING_COUNT] = {
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
  };
  static int8_t brightnessIndexes[2 * PIXEL_RING_COUNT] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
  };
  // Let's keep them the same color that they started with
  static uint16_t hues[2 * PIXEL_RING_COUNT] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
  };
  // Let's use our own sparkHue so that we can change the pixels more quickly
  static uint16_t sparkHue = 0;
  // Start with some zeroes so that we don't relight a pixel immediately
  const static uint8_t rippleBrightnesses[] = {0, 0, 0, 0, 0, 0, 4, 8, 16, 32, 48, 64, 64, 64};

  // So we pick a random LED to start making brighter
  const uint8_t led = random(2 * PIXEL_RING_COUNT);
  if (!increasing[led]) {
    increasing[led] = true;
    hues[led] = sparkHue;
    sparkHue += 1200;
  }

  for (uint8_t i = 0; i < COUNT_OF(increasing); ++i) {
    if (increasing[i]) {
      if (brightnessIndexes[i] < static_cast<int>(COUNT_OF(rippleBrightnesses))) {
        ++brightnessIndexes[i];
      } else {
        increasing[i] = false;
      }
      pixels->setPixelColor(i, pixels->ColorHSV(hues[i], 0xFF, rippleBrightnesses[brightnessIndexes[i]]));
    } else {
      if (brightnessIndexes[i] > 0) {
        --brightnessIndexes[i];
        pixels->setPixelColor(i, pixels->ColorHSV(hues[i], 0xFF, rippleBrightnesses[brightnessIndexes[i]]));
      }
    }
  }
  delay(50);
  pixels->show();
}


void newtonsCradle(Adafruit_NeoPixel* const pixels, const uint16_t hue) {
  static int8_t swingPosition = 0;
  static int8_t velocity = 7;

  const uint32_t color = pixels->ColorHSV(hue);
  const uint32_t cradleColor = pixels->ColorHSV(hue + 0x7FFF);
  const uint8_t center = 4;
  const uint8_t left = center - 1;
  const uint8_t right = center + 1;

  // Update the movement
  uint8_t movingLed;
  uint8_t staticLed;
  if (swingPosition < 0) {
    movingLed = left + swingPosition / 4;
    staticLed = right + 1;
    ++velocity;
  } else {
    movingLed = right + 1 + swingPosition / 4;
    staticLed = left - 1;
    --velocity;
  }
  swingPosition += velocity;

  movingLed = (movingLed + PIXEL_RING_COUNT) % PIXEL_RING_COUNT;

  // Draw the 3 cradle pixels
  pixels->setPixelColor(left, cradleColor);
  pixels->setPixelColor(center, cradleColor);
  pixels->setPixelColor(right, cradleColor);
  pixels->setPixelColor(staticLed, cradleColor);
  // Draw the moving pixels
  pixels->setPixelColor(movingLed, color);

  copyLens(pixels);
  pixels->show();
  delay(100);

  pixels->fill(0, 0, PIXEL_RING_COUNT * 2);
}


void rainbowSwirls(Adafruit_NeoPixel* pixels, uint16_t) {
  static uint16_t hue = 0;  // We use our own hue to make the colors rotate faster
  static uint8_t head = 0;
  const uint8_t brightness[] = {255, 192, 128, 96, 64, 48, 32, 24, 16, 8, 4, 2, 1};

  // I could probably optimize this loop so that I'm not going through the whole ring
  for (uint8_t i = 0; i < PIXEL_RING_COUNT; ++i) {
    const int8_t difference1 = head - i;
    if (0 <= difference1 && difference1 < static_cast<int>(COUNT_OF(brightness))) {
      const uint16_t adjustedHue = hue + difference1 * 4096;
      pixels->setPixelColor(i, pixels->ColorHSV(adjustedHue, 0xFF, brightness[difference1]));
    } else {
      const int8_t difference2 = PIXEL_RING_COUNT + difference1;
      if (0 <= difference2 && difference2 < static_cast<int>(COUNT_OF(brightness))) {
        const uint16_t adjustedHue = hue + difference2 * 64;
        pixels->setPixelColor(i, pixels->ColorHSV(adjustedHue, 0xFF, brightness[difference2]));
      }
    }
  }
  head = (head + 1) % PIXEL_RING_COUNT;
  hue += 1000;

  // Make the second lens swirl the other direction
  copyLens(pixels, PIXEL_RING_COUNT / 2);

  pixels->show();
  delay(50);
}


void randomSparks(Adafruit_NeoPixel* pixels, uint16_t hue) {
  // It's possible we might randomly choose the same LED multiple times per lens,
  // so this variable name isn't exactly accurate. I don't care though.
  const int LEDS_PER_LENS = 2;

  int used[2 * LEDS_PER_LENS] = {0};
  const uint32_t color = pixels->ColorHSV(hue);
  for (int i = 0; i < LEDS_PER_LENS; ++i) {
    const uint8_t led1 = random(PIXEL_RING_COUNT);
    pixels->setPixelColor(led1, color);
    used[i] = led1;
    const uint8_t led2 = random(PIXEL_RING_COUNT) + PIXEL_RING_COUNT;
    pixels->setPixelColor(led2, color);
    used[i + LEDS_PER_LENS] = led2;
  }
  pixels->show();
  delay(50);
  // And clear them
  for (int led : used) {
    pixels->setPixelColor(led, 0);
  }
}


void shimmer(Adafruit_NeoPixel* pixels, uint16_t hue) {
  // Start above 0 so that each light should be on a bit
  static const uint8_t brightness[] = {4, 8, 16, 32, 64, 128};
  static uint8_t values[PIXEL_RING_COUNT * 2] = {
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
  };

  for (int i = 0; i < 4; ++i) {
    const uint8_t pixel = random(PIXEL_RING_COUNT * 2);
    if (random(2) == 1) {
      if (values[pixel] < COUNT_OF(brightness)) {
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

  for (int i = 0; i < PIXEL_RING_COUNT * 2; ++i) {
    pixels->setPixelColor(i, pixels->ColorHSV(hue, 0xFF, brightness[values[i]]));
  }
  pixels->show();
  delay(50);
}


void spinnyWheels(Adafruit_NeoPixel* pixels, uint16_t) {
  static uint16_t hue = 0;  // We use our own hue to make the colors rotate faster
  static uint8_t offset = 0;  // Position of spinny eyes

  const uint32_t color = pixels->ColorHSV(hue);
  for (uint8_t i = 0; i < PIXEL_RING_COUNT; ++i) {
    uint32_t c = 0;
    if (((offset + i) & 0b111) < 2) {
      c = color;  // 4 pixels on...
    }
    pixels->setPixelColor(i, c);  // First eye
  }
  mirrorLens(pixels);
  pixels->show();
  ++offset;
  hue += 20;
  delay(50);
}


void swirls(Adafruit_NeoPixel* pixels, uint16_t hue) {
  static uint8_t head1 = 0;
  static const uint8_t brightness[] = {255, 128, 64, 32, 16, 8, 4, 2, 1};

  // I could probably optimize this loop so that I'm not going through the whole ring
  for (uint8_t i = 0; i < PIXEL_RING_COUNT; ++i) {
    const int8_t difference1 = head1 - i;
    if (0 <= difference1 && difference1 < static_cast<int>(COUNT_OF(brightness))) {
      pixels->setPixelColor(i, pixels->ColorHSV(hue, 0xFF, brightness[difference1]));
    } else {
      const int8_t difference2 = PIXEL_RING_COUNT + difference1;
      if (0 <= difference2 && difference2 < static_cast<int>(COUNT_OF(brightness))) {
        pixels->setPixelColor(i, pixels->ColorHSV(hue, 0xFF, brightness[difference2]));
      }
    }
  }
  head1 = (head1 + 1) % PIXEL_RING_COUNT;

  // Make the second lens swirl the other direction
  copyLens(pixels, PIXEL_RING_COUNT / 2);

  pixels->show();
  delay(50);
}


void copyLens(Adafruit_NeoPixel* const pixels, const uint8_t rotate) {
  // I didn't mount the lenses the same, so we need to fiddle a bit to get the copy effect
  const int OFFSET = 2;
  for (int i = 0; i < PIXEL_RING_COUNT; ++i) {
    pixels->setPixelColor((i + rotate + PIXEL_RING_COUNT - OFFSET) % PIXEL_RING_COUNT + PIXEL_RING_COUNT, pixels->getPixelColor(i));
  }
}


void mirrorLens(Adafruit_NeoPixel* const pixels, const uint8_t rotate) {
  // I didn't mount the lenses the same, so we need to fiddle a bit to get the mirror effect
  const int OFFSET = 2;
  for (int i = 0; i < PIXEL_RING_COUNT; ++i) {
    pixels->setPixelColor((PIXEL_RING_COUNT * 2 - i + rotate - OFFSET) % PIXEL_RING_COUNT + PIXEL_RING_COUNT, pixels->getPixelColor(i));
  }
}
