#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_timer.h>
#include <cassert>
#include <cstdint>
#include <limits>
#include <math.h>
#include <time.h>

#define PROGMEM

#define COUNT_OF(x) ((sizeof(x) / sizeof(0 [x])))

const int SPOKE_COUNT = 10;
const int RING_COUNT = 5;

const int WIDTH = 720;
const int HEIGHT = 480;
const int LED_WIDTH = 10;

SDL_Renderer *renderer = nullptr;

uint8_t sin8(uint8_t theta);
void setLed(int index, uint8_t red, uint8_t green, uint8_t blue,
            SDL_Renderer *renderer);
void setLed(int ring, int spoke, uint8_t red, uint8_t green, uint8_t blue,
            SDL_Renderer *renderer);
void setLedHue(int ring, int spoke, uint8_t hue, SDL_Renderer *renderer);
void setLedHue(int index, uint8_t hue, SDL_Renderer *renderer);
void setLedGrayscale(int ring, int spoke, uint8_t v, SDL_Renderer *renderer);
void setLedDoubleGrayscale(int ring, int spoke, uint8_t v,
                           SDL_Renderer *renderer);
void setLedPastel(int ring, int spoke, uint8_t v, SDL_Renderer *renderer);
void setLedFire(int ring, int spoke, uint8_t v, SDL_Renderer *renderer);
void hsvToRgb(uint8_t hue, uint8_t saturation, uint8_t value, uint8_t *red,
              uint8_t *green, uint8_t *blue);
typedef void(setLed_t(int, int, uint8_t, SDL_Renderer *));

#define MAX(a, b) ((a) > (b) ? (a) : (b))

const char *lightAll(int, SDL_Renderer *const renderer) {
  static uint8_t hue = 0;
  for (int i = 0; i < 50; ++i) {
    setLedHue(i, hue, renderer);
  }
  ++hue;
  return __func__;
}

const char *spinSingle(int, SDL_Renderer *const renderer) {
  static uint8_t hue = 0;
  static int spoke = 0;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    setLedHue(ring, spoke, hue, renderer);
  }
  ++hue;
  spoke = (spoke + 1) % SPOKE_COUNT;
  return __func__;
}

const char *outwardHue(int, SDL_Renderer *const renderer) {
  static uint8_t hue = 0;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      setLedHue(ring, spoke, hue - ring * 20, renderer);
    }
  }
  ++hue;
  return __func__;
}

const char *fastOutwardHue(int, SDL_Renderer *const renderer) {
  static uint8_t hue = 0;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      setLedHue(ring, spoke, hue - ring * 20, renderer);
    }
  }
  hue += 3;
  return __func__;
}

const char *inwardHue(int, SDL_Renderer *const renderer) {
  static uint8_t hue = 0;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      setLedHue(ring, spoke, hue + ring * 20, renderer);
    }
  }
  ++hue;
  return __func__;
}

const char *fastInwardHue(int, SDL_Renderer *const renderer) {
  static uint8_t hue = 0;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      setLedHue(ring, spoke, hue + ring * 20, renderer);
    }
  }
  hue += 3;
  return __func__;
}

const char *spiral(int, SDL_Renderer *const renderer) {
  static uint8_t hue = 0;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      setLedHue(RING_COUNT - 1 - ring, spoke, hue + ring * 20 + spoke * 25, renderer);
    }
  }
  hue += 3;
  return __func__;
}

const char *outwardRipple(int, SDL_Renderer *const renderer) {
  static uint8_t hue = 0;
  static uint8_t ripple = 0;
  uint8_t r, g, b;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      hsvToRgb(hue + ring * 15, 255, sin8(ripple - ring * 30), &r, &g, &b);
      setLed(ring, spoke, r, g, b, renderer);
    }
  }
  ++hue;
  ripple += 3;
  return __func__;
}

const char *outwardRippleHue(int, SDL_Renderer *const renderer) {
  static uint8_t hue = 0;
  static uint8_t ripple = 0;
  uint8_t r, g, b;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      hsvToRgb(hue + ring * 15 + spoke * (255 / SPOKE_COUNT), 255, sin8(ripple - ring * 30), &r, &g, &b);
      setLed(ring, spoke, r, g, b, renderer);
    }
  }
  hue += 2;
  ripple += 3;
  return __func__;
}

const char *singleSpiral(int, SDL_Renderer *const renderer) {
  static int spoke = 0;
  static uint8_t slow = 0;
  static uint8_t hue = 0;

  for (int ring = 0; ring < RING_COUNT; ++ring) {
    setLedHue(RING_COUNT - 1 - ring, (spoke + ring) % SPOKE_COUNT, hue, renderer);
  }

  slow = (slow + 1) % 2;
  if (slow == 0) {
    spoke = (spoke + 1) % SPOKE_COUNT;
    ++hue;
  }

  return __func__;
}

const char *blurredSpiral(int, SDL_Renderer *const renderer) {
  static int spoke = 0;
  static int slow = 0;
  static uint8_t hue = 0;

  uint8_t r, g, b;

  for (int ring = 0; ring < RING_COUNT; ++ring) {
    hsvToRgb(hue, 255, 255, &r, &g, &b);
    setLed(RING_COUNT - 1 - ring, (spoke + ring - 1 + SPOKE_COUNT) % SPOKE_COUNT, r / 2, g / 2, b / 2, renderer);
    setLed(RING_COUNT - 1 - ring, (spoke + ring) % SPOKE_COUNT, r, g, b, renderer);
    setLed(RING_COUNT - 1 - ring, (spoke + ring + 1) % SPOKE_COUNT, r / 2, g / 2, b / 2, renderer);
  }

  slow = (slow + 1) % 3;
  if (slow == 0) {
    spoke = (spoke + 1) % SPOKE_COUNT;
    ++hue;
  }

  return __func__;
}

const char *comets(int, SDL_Renderer *const renderer) {
  static uint8_t spokeHue[SPOKE_COUNT] = {0};
  static uint8_t spokeStart = 0;
  static uint8_t hue = 0;
  static int slow = 0;

  uint8_t r, g, b;

  for (int offset = 0; offset < RING_COUNT + 2; ++offset) {
    const int spoke = (spokeStart + offset) % SPOKE_COUNT;
    hsvToRgb(spokeHue[spoke], 255, 255, &r, &g, &b);
    setLed(RING_COUNT - offset - 1, spoke, r / 4, g / 4, b / 4, renderer);
    setLed(RING_COUNT - offset, spoke, r / 2, g / 2, b / 2, renderer);
    setLed(RING_COUNT - offset + 1, spoke, r, g, b, renderer);
  }

  slow = (slow + 1) % 3;
  if (slow == 0) {
    spokeHue[spokeStart] = hue;
    hue += 20;
    spokeStart = (spokeStart + 1) % SPOKE_COUNT;;
  }

  return __func__;
}

const char *snake(int, SDL_Renderer *const renderer) {
  static uint8_t hue = 0;
  static int index = 0;
  setLedHue(index, hue, renderer);
  ++hue;
  index = (index + 1) % (RING_COUNT * SPOKE_COUNT);
  return __func__;
}

int main() {
  bool shouldClose = false;
  uint8_t hue = 0;
  int animation_ms = 0;
  int time = 0;
  int animationIndex = 0;
  const char *(*animations[])(int, SDL_Renderer *) = {
      comets, snake, blurredSpiral, outwardRippleHue, singleSpiral, outwardRipple, spiral, lightAll, spinSingle, fastOutwardHue, fastInwardHue
  };

  // Returns zero on success else non-zero
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    printf("error initializing SDL: %s\n", SDL_GetError());
  }
  SDL_Window *window = SDL_CreateWindow(
      "vest", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SDL_SetWindowTitle(window, animations[0](time, renderer));

  // Animation loop
  while (!shouldClose) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    const int radius = 160;
    filledCircleRGBA(renderer, WIDTH / 2 + LED_WIDTH / 2,
                     HEIGHT / 2 + LED_WIDTH / 2, radius, 255, 255, 255, 255);
    filledCircleRGBA(renderer, WIDTH / 2 + LED_WIDTH / 2,
                     HEIGHT / 2 + LED_WIDTH / 2, radius - 2, 0, 0, 0, 255);
    animations[animationIndex](time, renderer);
    int delay_ms = 100;
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
      const char *name;
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
          shouldClose = true;
          goto exitDelay;

        case SDL_KEYDOWN:
          if (event.key.keysym.sym == SDLK_RIGHT) {
            goto nextAnimation;
          } else if (event.key.keysym.sym == SDLK_LEFT) {
            goto previousAnimation;
          }
          break;

        case SDL_MOUSEBUTTONDOWN:
          if (event.button.button == SDL_BUTTON_LEFT) {
            goto nextAnimation;
          } else if (event.button.button == SDL_BUTTON_RIGHT) {
            goto previousAnimation;
          }
          break;

        default:
          break;

        nextAnimation:
          animationIndex = (animationIndex + 1) % COUNT_OF(animations);
          name = animations[animationIndex](time, renderer);
          goto setTitle;

        previousAnimation:
          --animationIndex;
          if (animationIndex < 0) {
            animationIndex = COUNT_OF(animations) - 1;
          }
          name = animations[animationIndex](time, renderer);
          goto setTitle;

        setTitle:
          SDL_SetWindowTitle(window, name);
        }
      }
    }
  }
exitDelay:

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

const uint8_t b_m16_interleave[] = {0, 49, 49, 41, 90, 27, 117, 10};
uint8_t sin8(uint8_t theta) {
  uint8_t offset = theta;
  if (theta & 0x40) {
    offset = (uint8_t)255 - offset;
  }
  offset &= 0x3F; // 0..63

  uint8_t secoffset = offset & 0x0F; // 0..15
  if (theta & 0x40)
    secoffset++;

  uint8_t section = offset >> 4; // 0..3
  uint8_t s2 = section * 2;
  const uint8_t *p = b_m16_interleave;
  p += s2;
  uint8_t b = *p;
  p++;
  uint8_t m16 = *p;

  uint8_t mx = (m16 * secoffset) >> 4;

  int8_t y = mx + b;
  if (theta & 0x80)
    y = -y;

  y += 128;

  return y;
}

void setLedHue(const int index, const uint8_t h, SDL_Renderer *const renderer) {
  uint8_t red, green, blue;
  hsvToRgb(h, 255, 255, &red, &green, &blue);
  setLed(index, red, green, blue, renderer);
}

void setLedHue(const int ring, const int spoke, const uint8_t h,
               SDL_Renderer *const renderer) {
  uint8_t red, green, blue;
  hsvToRgb(h, 255, 255, &red, &green, &blue);
  setLed(ring, spoke, red, green, blue, renderer);
}

void setLedGrayscale(const int ring, const int spoke, const uint8_t v,
                     SDL_Renderer *const renderer) {
  const uint8_t value = sin8(v);
  setLed(ring, spoke, value, value, value, renderer);
}

void setLedDoubleGrayscale(const int ring, const int spoke, const uint8_t v,
                           SDL_Renderer *const renderer) {
  const uint8_t value = sin8(v * 2);
  setLed(ring, spoke, value, value, value, renderer);
}

void setLedPastel(const int ring, const int spoke, const uint8_t v,
                  SDL_Renderer *const renderer) {
  const uint8_t red = sin8(v);
  const uint8_t green = sin8(v + 2 * v / 3);
  const uint8_t blue = sin8(v + 4 * v / 3);
  setLed(ring, spoke, red, green, blue, renderer);
}

void setLedFire(int ring, int spoke, uint8_t v, SDL_Renderer *renderer) {
  const uint8_t red = v < 128 ? v * 2 : 255;
  const uint8_t green = v >= 128 ? (v - 128) * 2 : 0;
  setLed(ring, spoke, red, green, 0, renderer);
}

void setLed(const int index, const uint8_t red, const uint8_t green,
            const uint8_t blue, SDL_Renderer *const renderer) {
  const int spoke = index / 5;
  int ring;
  if (index % 10 < 5) {
    ring = index % 5;
  } else {
    ring = 4 - index % 5;
  }
  setLed(ring, spoke, red, green, blue, renderer);
}

void setLed(const int ring, const int spoke, const uint8_t red,
            const uint8_t green, const uint8_t blue,
            SDL_Renderer *const renderer) {
  const float SPACING = 20.0f;
  SDL_Rect rectangle;
  if (spoke >= 0 && spoke < SPOKE_COUNT) {
    if (ring >= 0 && ring < RING_COUNT) {
      const int xOffset =
          SPACING * (ring + 3) * sinf(spoke * (M_PI * 2.0 * 0.1));
      const int yOffset =
          SPACING * (ring + 3) * cosf(spoke * (M_PI * 2.0 * 0.1));
      // The ys are inverted
      rectangle.x = WIDTH / 2 + xOffset;
      rectangle.y = HEIGHT / 2 - yOffset;
      rectangle.w = LED_WIDTH;
      rectangle.h = LED_WIDTH;
      SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
      SDL_RenderFillRect(renderer, &rectangle);
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
