#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_timer.h>
#include <cassert>
#include <cmath>
#include <cstdint>

#include "../constants.hpp"

const int WIDTH = 720;
const int HEIGHT = 480;
const uint16_t PI_1_0 = 10430; // 1.0 / (3.14159 * 2) * 65536

SDL_Renderer *renderer = nullptr;

int16_t sin16(uint16_t theta);
uint8_t sqrt16(uint16_t value);
int16_t cos16(uint16_t theta);
void setLed(int x, int y, uint8_t hue, SDL_Renderer *renderer);
void hsvToRgb(uint8_t hue, uint8_t saturation, uint8_t value, uint8_t *red,
              uint8_t *green, uint8_t *blue);

// https://lodev.org/cgtutor/plasma.html
int plasma1(int time, SDL_Renderer *const renderer) {
  for (int x = 0; x < LED_COLUMN_COUNT; ++x) {
    for (int y = 0; y < LED_ROW_COUNT; ++y) {
      const uint8_t p1 = 128 + (sin16(x * (PI_1_0 / 4))) / 256;
      const uint8_t p2 = 128 + (sin16(y * (PI_1_0 / 2) + time / 4)) / 256;
      const uint8_t p3 = 128 + (sin16((x + y) * (PI_1_0 / 8) + time / 8)) / 256;
      const uint8_t p4 = 128 + (sin16(sqrt16(x * x + y * y) * 1000 + time)) / 256;
      const uint8_t hue = (p1 + p2 + p3 + p4) / 4;
      setLed(x, y, hue, renderer);
    }
  }
  return 100;
};

int plasma2(int time, SDL_Renderer *const renderer) {
  for (int x = 0; x < LED_COLUMN_COUNT; ++x) {
    for (int y = 0; y < LED_ROW_COUNT; ++y) {
      const int xOffset = (x - LED_COLUMN_COUNT) / 2;
      const int yOffset = (y - LED_ROW_COUNT) / 2;
      const uint8_t hue = static_cast<uint8_t>(
          (128 + (sin16(x * (PI_1_0 / 16))) / 256 + 128 +
           (sin16(y * (PI_1_0 / 32))) / 256 + 128 +
           (sin16(sqrt16((xOffset * xOffset) + (yOffset * yOffset)) * 1000)) /
               256 +
           128 + (sin16(sqrt16(x * x + y * y) * 1000 + time)) / 256) /
          4);

      setLed(x, y, hue, renderer);
    }
  }
  return 100;
}

int plasma3(int time, SDL_Renderer *const renderer) {
  for (int x = 0; x < LED_COLUMN_COUNT; ++x) {
    for (int y = 0; y < LED_ROW_COUNT; ++y) {
      const uint8_t hue = static_cast<uint8_t>(
          (128 + (sin16(x * (PI_1_0 / 4))) / 256 + 128 +
           (sin16(y * (PI_1_0 / 2) + time / 4)) / 256 + 128 +
           (sin16((x + y) * (PI_1_0 / 8) + time / 8)) / 256 + 128 +
           (sin16(sqrt16(x * x + y * y) * 1000 + time)) / 256) /
          4);

      setLed(x, y, hue, renderer);
    }
  }
  return 100;
};

int plasma4(int time, SDL_Renderer *const renderer) {
  for (int x = 0; x < LED_COLUMN_COUNT; ++x) {
    for (int y = 0; y < LED_ROW_COUNT; ++y) {
      const uint8_t hue = static_cast<uint8_t>(
          (128 + (sin16(x * (PI_1_0 / 4))) / 256 + 128 +
           (sin16(y * (PI_1_0 / 2) + time / 4)) / 256 + 128 +
           (sin16((x + y) * (PI_1_0 / 8) + time / 8)) / 256 + 128 +
           (sin16(sqrt16(x * x + y * y) * 1000 + time)) / 256) /
          4);

      setLed(x, y, hue, renderer);
    }
  }
  return 100;
};

int plasma5(int time, SDL_Renderer *const renderer) {
  for (int x = 0; x < LED_COLUMN_COUNT; ++x) {
    for (int y = 0; y < LED_ROW_COUNT; ++y) {
      const uint8_t hue = static_cast<uint8_t>(
          (128 + (sin16(x * (PI_1_0 / 4))) / 256 + 128 +
           (sin16(y * (PI_1_0 / 2) + time / 4)) / 256 + 128 +
           (sin16((x + y) * (PI_1_0 / 8) + time / 8)) / 256 + 128 +
           (sin16(sqrt16(x * x + y * y) * 1000 + time)) / 256) /
          4);

      setLed(x, y, hue, renderer);
    }
  }
  return 100;
};

int plasma6(int time, SDL_Renderer *const renderer) {
  for (int x = 0; x < LED_COLUMN_COUNT; ++x) {
    for (int y = 0; y < LED_ROW_COUNT; ++y) {
      const uint8_t hue = static_cast<uint8_t>(
          (128 + (sin16(x * (PI_1_0 / 4))) / 256 + 128 +
           (sin16(y * (PI_1_0 / 2) + time / 4)) / 256 + 128 +
           (sin16((x + y) * (PI_1_0 / 8) + time / 8)) / 256 + 128 +
           (sin16(sqrt16(x * x + y * y) * 1000 + time)) / 256) /
          4);

      setLed(x, y, hue, renderer);
    }
  }
  return 100;
};


int main() {

  // Returns zero on success else non-zero
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    printf("error initializing SDL: %s\n", SDL_GetError());
  }
  SDL_Window *win = SDL_CreateWindow("vest", SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

  renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

  bool shouldClose = false;
  uint8_t hue = 0;
  int animation_ms = 0;
  int time;
  int animationIndex = 0;
  int (*animations[])(int, SDL_Renderer*) = {&plasma1, &plasma2, &plasma3, nullptr};

  // Animation loop
  while (!shouldClose) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    int delay_ms = animations[animationIndex](time, renderer);
    time += 1000;
    ++hue;
    SDL_RenderPresent(renderer);

    while (delay_ms > 0) {
      // 60 FPS
      SDL_Delay(1000 / 60);
      delay_ms -= 60;

      animation_ms += 1000 / 60;

      SDL_Event event;
      // Events management
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
          shouldClose = true;
          goto exitDelay;

        case SDL_KEYDOWN:
          ++animationIndex;
          if (animations[animationIndex] == nullptr) {
            animationIndex = 0;
          }

        default:
          break;
        }
      }
    }
  }
exitDelay:

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(win);
  SDL_Quit();

  return 0;
}

int16_t sin16(uint16_t theta) {
  static const uint16_t base[] = {0,     6393,  12539, 18204,
                                  23170, 27245, 30273, 32137};
  static const uint8_t slope[] = {49, 48, 44, 38, 31, 23, 14, 4};

  uint16_t offset = (theta & 0x3FFF) >> 3; // 0..2047
  if (theta & 0x4000) {
    offset = 2047 - offset;
  }

  uint8_t section = offset / 256; // 0..7
  uint16_t b = base[section];
  uint8_t m = slope[section];

  uint8_t secoffset8 = static_cast<uint8_t>(offset) / 2;

  uint16_t mx = m * secoffset8;
  int16_t y = mx + b;

  if (theta & 0x8000) {
    y = -y;
  }

  return y;
}

uint8_t sqrt16(const uint16_t value) {
  uint8_t x = 0;
  if (value > 65536 / 3) {
    return 255;
  }
  while (x * x < value) {
    ++x;
  }
  return x;
}

int16_t cos16(uint16_t theta) { return sin16(theta + 65536 / 4); }

void setLed(const int x, const int y, const uint8_t hue,
            SDL_Renderer *const renderer) {
  SDL_Rect rectangle;
  const int multiplier = 25;
  if (x >= 0 && x < LED_COLUMN_COUNT) {
    if (y >= 0 && y < LED_ROW_COUNT) {
      const int index = LED_STRIPS[x][y];
      if (index != UNUSED_LED) {
        // The ys are inverted
        rectangle.x = x * multiplier;
        rectangle.y = LED_ROW_COUNT * multiplier - y * multiplier;
        rectangle.w = multiplier;
        rectangle.h = multiplier;
        uint8_t red, green, blue;
        hsvToRgb(hue, 255, 255, &red, &green, &blue);
        SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
        SDL_RenderFillRect(renderer, &rectangle);
      }
    }
  }
}

void hsvToRgb(const uint8_t hue, const uint8_t saturation, const uint8_t value,
              uint8_t *const red, uint8_t *const green, uint8_t *const blue) {
  unsigned char region, remainder, p, q, t;

  if (saturation == 0) {
    *red = value;
    *green = value;
    *blue = value;
    return;
  }

  region = hue / 43;
  remainder = (hue - (region * 43)) * 6;

  p = (value * (255 - saturation)) >> 8;
  q = (value * (255 - ((saturation * remainder) >> 8))) >> 8;
  t = (value * (255 - ((saturation * (255 - remainder)) >> 8))) >> 8;

  switch (region) {
  case 0:
    *red = value;
    *green = t;
    *blue = p;
    break;
  case 1:
    *red = q;
    *green = value;
    *blue = p;
    break;
  case 2:
    *red = p;
    *green = value;
    *blue = t;
    break;
  case 3:
    *red = p;
    *green = q;
    *blue = value;
    break;
  case 4:
    *red = t;
    *green = p;
    *blue = value;
    break;
  default:
    *red = value;
    *green = p;
    *blue = q;
    break;
  }
}
