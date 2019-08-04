/** Simpler animations go here */

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <cstdint>

#include "constants.hpp"

static void showNumber(Adafruit_NeoPixel* pixels, uint32_t number, const uint32_t color);

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


void swirls(Adafruit_NeoPixel* pixels, uint16_t hue) {
  static uint8_t head1 = 0;
  static uint8_t head2 = 3;
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
  for (uint8_t i = 0; i < PIXEL_RING_COUNT; ++i) {
    const int8_t difference1 = head2 - i;
    if (0 <= difference1 && difference1 < static_cast<int>(COUNT_OF(brightness))) {
      pixels->setPixelColor(PIXEL_RING_COUNT * 2 - 1 - i, pixels->ColorHSV(hue, 0xFF, brightness[difference1]));
    } else {
      const int8_t difference2 = PIXEL_RING_COUNT + difference1;
      if (0 <= difference2 && difference2 < static_cast<int>(COUNT_OF(brightness))) {
        pixels->setPixelColor(PIXEL_RING_COUNT * 2 - 1 - i, pixels->ColorHSV(hue, 0xFF, brightness[difference2]));
      }
    }
  }
  head2 = (head2 + 1) % PIXEL_RING_COUNT;

  pixels->show();
  delay(50);
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


void randomSparks(Adafruit_NeoPixel* pixels, uint16_t hue) {
  const uint32_t color = pixels->ColorHSV(hue);
  const uint8_t led1 = random(PIXEL_RING_COUNT);
  pixels->setPixelColor(led1, color);
  const uint8_t led2 = random(PIXEL_RING_COUNT) + PIXEL_RING_COUNT;
  pixels->setPixelColor(led2, color);
  pixels->show();
  delay(20);
  pixels->setPixelColor(led1, 0);
  pixels->setPixelColor(led2, 0);
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
    pixels->setPixelColor(PIXEL_RING_COUNT * 2 - 1 - i, c);  // Second eye (flipped)
  }
  pixels->show();
  ++offset;
  hue += 20;
  delay(50);
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
