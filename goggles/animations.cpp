/** Simpler animations go here */

#include <Arduino.h>
#include <cstdint>
#include <FastLED.h>

#include "constants.hpp"
#include "functions.hpp"

extern CRGB pixels[2 * PIXEL_RING_COUNT];

static void showNumber(uint32_t number, const CRGB& color);
static void copyLens(uint8_t rotate = 0);
static void mirrorLens(uint8_t rotate = 0);

void binaryClock(uint8_t hue) {
  // Do a binary shift instead of integer division because of speed and code size.
  // It's a little less precise, but who cares.
  // Show 1/4 seconds instead of full seconds because it's more interesting. It
  // updates a lot faster and the other lens lights up faster.
  auto now = millis() >> 8;
  const auto color = gbChsv(hue, 0xFF, 0xFF);
  showNumber(now, color);
  delay(100);
}


static void showNumber(uint32_t number, const CRGB& color) {
  uint8_t counter = 0;
  while (number > 0) {
    if (number & 1) {
      pixels[counter] = color;
    } else {
      pixels[counter] = CRGB::Black;
    }
    number >>= 1;
    counter += 2;
  }
  FastLED.show();
}


void lookAround(const uint8_t hue) {
  enum class BlinkState {
    DOWN,
    UP,
    BLINKED
  };
  static uint8_t target;
  static uint8_t current;
  static decltype(millis()) startTime_ms;
  static BlinkState blinkState;
  static uint8_t blinkOffset;
  static uint8_t blinkCount;
  extern bool reset;
  const int OFFSET = 4;  // Offset so the blink starts at the top and bottom of the lens
  const int MOVE_1 = 255;
  const int MOVE_2 = 254;

  if (reset) {
    startTime_ms = millis();
    current = random(PIXEL_RING_COUNT);
    target = MOVE_1;
    blinkState = BlinkState::DOWN;
    blinkOffset = 0;
    blinkCount = 0;
    fill_solid(&pixels[0], PIXEL_RING_COUNT * 2, CRGB::Black);
    pixels[current] = gbChsv(hue, 0xFF, 0xFF);
    copyLens();
    FastLED.show();
    reset = false;
  }

  const auto timeOffset_ms = millis() - startTime_ms;
  if (timeOffset_ms < 1000) {
    delay(100);
    // We return so that the hue can still change
    return;
  } else if (timeOffset_ms < 3000) {
    if (target == MOVE_1) {
      target = (current + 4 - random(8) + PIXEL_RING_COUNT / 2) % PIXEL_RING_COUNT;
    }
    if (target != MOVE_2 && current != target) {
      pixels[current] = CRGB::Black;
      // TODO: Go in the closer direction, or a random direction
      current = (current + 1) % PIXEL_RING_COUNT;
      pixels[current] = gbChsv(hue, 0xFF, 0xFF);
      copyLens();
      FastLED.show();
    } else {
      target = MOVE_2;
    }
    delay(50);
  } else if (timeOffset_ms < 5000) {
    if (target == MOVE_2) {
      target = (current + 4 - random(8) + PIXEL_RING_COUNT / 2) % PIXEL_RING_COUNT;
    }
    if (current != target) {
      pixels[current] = CRGB::Black;
      // TODO: Go in the closer direction, or a random direction
      current = (current - 1 + PIXEL_RING_COUNT) % PIXEL_RING_COUNT;
      pixels[current] = gbChsv(hue, 0xFF, 0xFF);
      copyLens();
      FastLED.show();
    }
    delay(50);
  } else {
    fill_solid(&pixels[0], PIXEL_RING_COUNT, CRGB::Black);
    pixels[target] = gbChsv(hue, 0xFF, 0xFF);
    const auto blinkColor = gbChsv(hue + 0x7FFF, 0xFF, 0xFF / 4);
    switch (blinkState) { 
      case BlinkState::DOWN:
        for (int i = 0; i < blinkOffset; ++i) {
          pixels[(i + OFFSET) % PIXEL_RING_COUNT] = blinkColor;
          pixels[((PIXEL_RING_COUNT / 2 - i + OFFSET) % PIXEL_RING_COUNT)] = blinkColor;
          pixels[((PIXEL_RING_COUNT / 2 + i + OFFSET) % PIXEL_RING_COUNT)] = blinkColor;
          pixels[((PIXEL_RING_COUNT - i + OFFSET) % PIXEL_RING_COUNT)] = blinkColor;
        }
        ++blinkOffset;
        if (blinkOffset == PIXEL_RING_COUNT / 4) {
          blinkState = BlinkState::UP;
        }
        break;
      case BlinkState::UP:
        for (int i = 0; i < blinkOffset; ++i) {
          pixels[((i + OFFSET) % PIXEL_RING_COUNT)] = blinkColor;
          pixels[((PIXEL_RING_COUNT / 2 - i + OFFSET) % PIXEL_RING_COUNT)] = blinkColor;
          pixels[((PIXEL_RING_COUNT / 2 + i + OFFSET) % PIXEL_RING_COUNT)] = blinkColor;
          pixels[((PIXEL_RING_COUNT - i + OFFSET) % PIXEL_RING_COUNT)] = blinkColor;
        }
        --blinkOffset;
        if (blinkOffset == 0) {
          ++blinkCount;
          if (blinkCount == 2) {
            blinkState = BlinkState::BLINKED;
          } else {
            blinkState = BlinkState::DOWN;
          }
        }
        break;
      case BlinkState::BLINKED:
        break;
    }

    copyLens();
    FastLED.show();
    delay(50);
  }
}


void fadingSparks(uint8_t) {
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
      if (brightnessIndexes[i] < static_cast<int>(COUNT_OF(rippleBrightnesses)) - 1) {
        ++brightnessIndexes[i];
      } else {
        increasing[i] = false;
      }
      pixels[i] = gbChsv(hues[i], 0xFF, rippleBrightnesses[brightnessIndexes[i]]);
    } else {
      if (brightnessIndexes[i] > 0) {
        --brightnessIndexes[i];
        pixels[i] = gbChsv(hues[i], 0xFF, rippleBrightnesses[brightnessIndexes[i]]);
      }
    }
  }
  delay(50);
  FastLED.show();
}


void newtonsCradle(const uint8_t hue) {
  static int8_t swingPosition = 0;
  static int8_t velocity = 7;

  const auto color = gbChsv(hue, 0xFF, 0xFF);
  const auto cradleColor = gbChsv(hue + 0x7FFF, 0xFF, 0xFF);
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
  pixels[left] = cradleColor;
  pixels[center] = cradleColor;
  pixels[right] = cradleColor;
  pixels[staticLed] = cradleColor;
  // Draw the moving pixels
  pixels[movingLed] = color;

  copyLens();
  FastLED.show();
  delay(100);

  fill_solid(&pixels[0], PIXEL_RING_COUNT * 2, CRGB::Black);
}


void rainbowSwirls(uint8_t) {
  static uint16_t hue = 0;  // We use our own hue to make the colors rotate faster
  static uint8_t head = 0;
  const uint8_t brightness[] = {255, 192, 128, 96, 64, 48, 32, 24, 16, 8, 4, 2, 1};

  // I could probably optimize this loop so that I'm not going through the whole ring
  for (uint8_t i = 0; i < PIXEL_RING_COUNT; ++i) {
    const int8_t difference1 = head - i;
    if (0 <= difference1 && difference1 < static_cast<int>(COUNT_OF(brightness))) {
      const uint16_t adjustedHue = hue + difference1 * 4096;
      pixels[i] = gbChsv(adjustedHue, 0xFF, brightness[difference1]);
    } else {
      const int8_t difference2 = PIXEL_RING_COUNT + difference1;
      if (0 <= difference2 && difference2 < static_cast<int>(COUNT_OF(brightness))) {
        const uint16_t adjustedHue = hue + difference2 * 64;
        pixels[i] = gbChsv(adjustedHue, 0xFF, brightness[difference2]);
      }
    }
  }
  head = (head + 1) % PIXEL_RING_COUNT;
  hue += 1000;

  // Make the second lens swirl the other direction
  copyLens(PIXEL_RING_COUNT / 2);

  FastLED.show();
  delay(50);
}


void randomSparks(uint8_t hue) {
  // It's possible we might randomly choose the same LED multiple times per lens,
  // so this variable name isn't exactly accurate. I don't care though.
  const int LEDS_PER_LENS = 2;

  int used[2 * LEDS_PER_LENS] = {0};
  const auto color = gbChsv(hue, 0xFF, 0xFF);
  for (int i = 0; i < LEDS_PER_LENS; ++i) {
    const uint8_t led1 = random(PIXEL_RING_COUNT);
    pixels[led1] = color;
    used[i] = led1;
    const uint8_t led2 = random(PIXEL_RING_COUNT) + PIXEL_RING_COUNT;
    pixels[led2] = color;
    used[i + LEDS_PER_LENS] = led2;
  }
  FastLED.show();
  delay(50);
  // And clear them
  for (int led : used) {
    pixels[led] = CRGB::Black;
  }
}


void shimmer(uint8_t hue) {
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
    pixels[i] = gbChsv(hue, 0xFF, brightness[values[i]]);
  }
  FastLED.show();
  delay(50);
}


void spinnyWheels(uint8_t) {
  static uint16_t hue = 0;  // We use our own hue to make the colors rotate faster
  static uint8_t offset = 0;  // Position of spinny eyes

  const auto color = gbChsv(hue, 0xFF, 0xFF);
  for (uint8_t i = 0; i < PIXEL_RING_COUNT; ++i) {
    CRGB c = CRGB::Black;
    if (((offset + i) & 0b111) < 2) {
      c = color;  // 4 pixels on...
    }
    pixels[i] = c;  // First eye
  }
  mirrorLens();
  FastLED.show();
  ++offset;
  hue += 20;
  delay(50);
}


void swirls(uint8_t hue) {
  static uint8_t head1 = 0;
  static const uint8_t brightness[] = {255, 128, 64, 32, 16, 8, 4, 2, 1};

  // I could probably optimize this loop so that I'm not going through the whole ring
  for (uint8_t i = 0; i < PIXEL_RING_COUNT; ++i) {
    const int8_t difference1 = head1 - i;
    if (0 <= difference1 && difference1 < static_cast<int>(COUNT_OF(brightness))) {
      pixels[i] = gbChsv(hue, 0xFF, brightness[difference1]);
    } else {
      const int8_t difference2 = PIXEL_RING_COUNT + difference1;
      if (0 <= difference2 && difference2 < static_cast<int>(COUNT_OF(brightness))) {
        pixels[i] = gbChsv(hue, 0xFF, brightness[difference2]);
      }
    }
  }
  head1 = (head1 + 1) % PIXEL_RING_COUNT;

  // Make the second lens swirl the other direction
  copyLens(PIXEL_RING_COUNT / 2);

  FastLED.show();
  delay(50);
}


void circularWipe(const uint8_t hue) {
  static uint8_t head = 0;
  static CRGB currentColor = CRGB(0xFF0000);
  static CRGB previousColor = CRGB(0x00FF00);

  fill_solid(&pixels[0], head, currentColor);
  fill_solid(&pixels[head], PIXEL_RING_COUNT - head, previousColor);
  if (head < PIXEL_RING_COUNT) {
    ++head;
  } else {
    head = 0;
    previousColor = currentColor;
    currentColor = gbChsv(hue, 0xFF, 0xFF);
  }
  mirrorLens();
  FastLED.show();
  delay(50);
}


void pacMan(uint8_t) {
  enum class PacManState {
    RUNNING,
    CHASING_BLUE,
    CHASING_WHITE,
  };
  const uint8_t powerPillPosition = PIXEL_RING_COUNT - 1;
  const auto yellow = 0xFFFF00;
  const auto white = 0xFFFFFF;
  const auto blue = 0x0000FF;
  const uint32_t ghostColors[4] = {0xFF0000, 0xFFB8FF, 0x00FFFF, 0xFFB852};

  static PacManState state = PacManState::RUNNING;
  static uint8_t pacManPosition = 9;
  static uint8_t ghostPosition = pacManPosition - 2;
  static bool ghostAlive[COUNT_OF(ghostColors)];

  extern bool reset;
  if (reset) {
    state = PacManState::RUNNING;
    pacManPosition = 9;
    ghostPosition = pacManPosition - 2;
    for (auto &ga : ghostAlive) {
      ga = true;
    }
  }

  // Redraw
  fill_solid(&pixels[0], PIXEL_RING_COUNT, CRGB::Black);
  if (state == PacManState::RUNNING) {
    pixels[powerPillPosition] = white;
  }
  for (uint8_t i = 0; i < COUNT_OF(ghostColors); ++i) {
    if (ghostAlive[i]) {
      uint32_t ghostColor;
      switch (state) {
        case PacManState::RUNNING:
          ghostColor = ghostColors[i];
          break;
        case PacManState::CHASING_WHITE:
          ghostColor = white;
          break;
        case PacManState::CHASING_BLUE:
          ghostColor = blue;
          break;
         default:
          // Do this just to appease a compiler error about uninitialized use.
          // This shouldn't happen.
          ghostColor = ghostColors[i];
          break;
      }
      const uint8_t updatedGhostPosition = ghostPosition - (i * 2);
      pixels[updatedGhostPosition] = ghostColor;
    }
  }
  // Draw Pac-Man last, in case he's on top of something
  pixels[pacManPosition] = yellow;

  mirrorLens();
  FastLED.show();
  delay(100);

  switch (state) {
    case PacManState::RUNNING:
      if (pacManPosition == powerPillPosition) {
        state = PacManState::CHASING_BLUE;
      } else {
        ++pacManPosition;
        ++ghostPosition;
      }
      break;
    case PacManState::CHASING_BLUE:
      --pacManPosition;
      for (uint8_t i = 0; i < COUNT_OF(ghostColors); ++i) {
        const uint8_t thisGhost = ghostPosition - (i * 2);
        if (pacManPosition == thisGhost) {
          ghostAlive[i] = false;
          break;
        }
      }
      state = PacManState::CHASING_BLUE;
      break;
    case PacManState::CHASING_WHITE:
      --pacManPosition;
      --ghostPosition;
      state = PacManState::CHASING_WHITE;
      break;
  }
  // Once we enter the CHASING states, we never leave (until reset) so just
  // have Pac-Man spin in circles
  if (pacManPosition > PIXEL_RING_COUNT) {
    pacManPosition = PIXEL_RING_COUNT - 1;
  }
}


void copyLens(const uint8_t rotate) {
  // I didn't mount the lenses the same, so we need to fiddle a bit to get the copy effect
  const int OFFSET = 2;
  for (int i = 0; i < PIXEL_RING_COUNT; ++i) {
    pixels[((i + rotate + PIXEL_RING_COUNT - OFFSET) % PIXEL_RING_COUNT + PIXEL_RING_COUNT)] = pixels[i];
  }
}


void mirrorLens(const uint8_t rotate) {
  // I didn't mount the lenses the same, so we need to fiddle a bit to get the mirror effect
  const int OFFSET = 2;
  for (int i = 0; i < PIXEL_RING_COUNT; ++i) {
    pixels[((PIXEL_RING_COUNT * 2 - i + rotate - OFFSET) % PIXEL_RING_COUNT + PIXEL_RING_COUNT)] = pixels[i];
  }
}
