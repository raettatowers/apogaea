#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_timer.h>
#include <cassert>
#include <cstdint>
#include <math.h>
#include <time.h>

#define COUNT_OF(x) ((sizeof(x) / sizeof(0 [x])))

const int SPOKE_COUNT = 9;
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

struct AnimationResult {
  const char *functionName;
  int delay_ms;
  AnimationResult(const char *f, int d) : functionName(f), delay_ms(d) {}
};

static AnimationResult lightAll(SDL_Renderer *const renderer) {
  static uint8_t hue = 0;
  for (int i = 0; i < 50; ++i) {
    setLedHue(i, hue, renderer);
  }
  ++hue;
  return AnimationResult(__func__, 100);
}

static AnimationResult spinSingle(SDL_Renderer *const renderer) {
  static uint8_t hue = 0;
  static int spoke = 0;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    setLedHue(ring, spoke, hue, renderer);
  }
  ++hue;
  spoke = (spoke + 1) % SPOKE_COUNT;
  return AnimationResult(__func__, 100);
}

static AnimationResult fastOutwardHue(SDL_Renderer *const renderer) {
  static uint8_t hue = 0;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      setLedHue(ring, spoke, hue - ring * 20, renderer);
    }
  }
  hue += 3;
  return AnimationResult(__func__, 100);
}

static AnimationResult fastInwardHue(SDL_Renderer *const renderer) {
  static uint8_t hue = 0;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      setLedHue(ring, spoke, hue + ring * 20, renderer);
    }
  }
  hue += 3;
  return AnimationResult(__func__, 100);
}

static AnimationResult spiral(SDL_Renderer *const renderer) {
  static uint8_t hue = 0;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      setLedHue(RING_COUNT - 1 - ring, SPOKE_COUNT - 1 - spoke,
                hue + ring * 20 + spoke * 25, renderer);
    }
  }
  hue += 3;
  return AnimationResult(__func__, 100);
}

static AnimationResult outwardRipple(SDL_Renderer *const renderer) {
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
  return AnimationResult(__func__, 100);
}

static AnimationResult outwardRippleHue(SDL_Renderer *const renderer) {
  static uint8_t hue = 0;
  static uint8_t ripple = 0;
  uint8_t r, g, b;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      hsvToRgb(hue + ring * 15 + spoke * (255 / SPOKE_COUNT), 255,
               sin8(ripple - ring * 30), &r, &g, &b);
      setLed(ring, spoke, r, g, b, renderer);
    }
  }
  hue += 2;
  ripple += 3;
  return AnimationResult(__func__, 100);
}

static AnimationResult singleSpiral(SDL_Renderer *const renderer) {
  static int spoke = 0;
  static uint8_t hue = 0;

  for (int ring = 0; ring < RING_COUNT; ++ring) {
    setLedHue(RING_COUNT - 1 - ring, (spoke + ring) % SPOKE_COUNT, hue,
              renderer);
  }

  spoke = (spoke + 1) % SPOKE_COUNT;
  ++hue;

  return AnimationResult(__func__, 400);
}

static AnimationResult blurredSpiral(SDL_Renderer *const renderer) {
  const int length = 5;
  const int brightnesses[length] = {255 / 4, 255 / 2, 255, 255 / 2, 255 / 4};

  static int currentSpoke = 0;
  static uint8_t currentHue = 0;
  static int8_t starts[SPOKE_COUNT] = {0};

  uint8_t r, g, b;

  for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
    for (int offset = 0; offset < length; ++offset) {
      hsvToRgb(currentHue, 255, brightnesses[offset], &r, &g, &b);
      // Rely on the checks in setLed to not step out of bounds
      setLed(starts[spoke] + offset, spoke, r, g, b, renderer);
    }
    ++starts[spoke];
  }

  starts[currentSpoke] = -length + 1;
  currentSpoke = (currentSpoke + 1) % SPOKE_COUNT;
  ++currentHue;

  return AnimationResult(__func__, 400);
}

static AnimationResult blurredSpiralHues(SDL_Renderer *const renderer) {
  const int length = 5;
  const int brightnesses[length] = {255 / 4, 255 / 2, 255, 255 / 2, 255 / 4};

  static int currentSpoke = 0;
  static uint8_t currentHue = 0;
  static uint8_t hues[SPOKE_COUNT];
  static int8_t starts[SPOKE_COUNT] = {0};

  uint8_t r, g, b;

  for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
    for (int offset = 0; offset < length; ++offset) {
      hsvToRgb(hues[spoke], 255, brightnesses[offset], &r, &g, &b);
      // Rely on the checks in setLed to not step out of bounds
      setLed(starts[spoke] + offset, spoke, r, g, b, renderer);
    }
    ++starts[spoke];
  }

  starts[currentSpoke] = -length + 1;
  hues[currentSpoke] = currentHue;
  currentSpoke = (currentSpoke + 1) % SPOKE_COUNT;
  currentHue += 10;

  return AnimationResult(__func__, 400);
}

static AnimationResult orbit(SDL_Renderer *const renderer) {
  const int startSpeed = 13;
  const int divisor = 16;
  const int fade = 5;

  static int currentSpoke = 0;
  static int position = -startSpeed;
  static int speed = startSpeed;
  static uint8_t hue = 0;
  static uint8_t brightness[RING_COUNT][SPOKE_COUNT] = {0};

  uint8_t r, g, b;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      if (brightness[ring][spoke] == 255) {
        brightness[ring][spoke] = 128;
      } else if (brightness[ring][spoke] > fade) {
        brightness[ring][spoke] -= fade;
      } else {
        brightness[ring][spoke] = 0;
      }
      hsvToRgb(hue, 255, brightness[ring][spoke], &r, &g, &b);
      setLed(ring, spoke, r, g, b, renderer);
    }
  }

  // The ball should always be maximum brightness
  setLedHue(position / divisor, currentSpoke, hue, renderer);
  brightness[position / divisor][currentSpoke] = 255;
  position += speed;

  if (position < 0) {
    position = 0;
    speed = startSpeed;
    currentSpoke = (currentSpoke + (SPOKE_COUNT / 2) + 1) % SPOKE_COUNT;
  }
  --speed;
  ++hue;

  return AnimationResult(__func__, 150);
}

static AnimationResult triadOrbits(SDL_Renderer *const renderer) {
  const int startSpeed = 13;
  const int divisor = 16;
  const int fade = 10;
  const int steps = 3;

  static int position = -startSpeed;
  static int speed = startSpeed;
  static int currentSpoke = 0;
  static uint8_t hue = 0;
  static uint8_t brightness[RING_COUNT][SPOKE_COUNT] = {0};

  uint8_t r, g, b;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      if (brightness[ring][spoke] == 255) {
        brightness[ring][spoke] = 128;
      } else if (brightness[ring][spoke] > fade) {
        brightness[ring][spoke] -= fade;
      } else {
        brightness[ring][spoke] = 0;
      }
      hsvToRgb(hue, 255, brightness[ring][spoke], &r, &g, &b);
      setLed(ring, spoke, r, g, b, renderer);
    }
  }

  // The ball should always be maximum brightness
  for (int spoke = currentSpoke; spoke < SPOKE_COUNT; spoke += 3) {
    setLedHue(position / divisor, spoke, hue, renderer);
    brightness[position / divisor][spoke] = 255;
  }
  position += speed;

  if (position < 0) {
    position = 0;
    speed = startSpeed;
    currentSpoke = (currentSpoke + 1) % steps;
    hue += 50;
  }
  --speed;
  ++hue;

  return AnimationResult(__func__, 150);
}

static AnimationResult pendulum(SDL_Renderer *const renderer) {
  const int divisor = 16;
  const int fade = 10;

  static int position = divisor + divisor / 2;
  static int speed = 0;
  static uint8_t hue = 0;
  static uint8_t brightness[RING_COUNT][SPOKE_COUNT] = {0};

  uint8_t r, g, b;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      if (brightness[ring][spoke] == 255) {
        brightness[ring][spoke] = 128;
      } else if (brightness[ring][spoke] > fade) {
        brightness[ring][spoke] -= fade;
      } else {
        brightness[ring][spoke] = 0;
      }
      hsvToRgb(hue, 255, brightness[ring][spoke], &r, &g, &b);
      setLed(ring, spoke, r, g, b, renderer);
    }
  }

  // The pendulum should always be maximum brightness
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    setLedHue(ring, position / divisor, hue, renderer);
    brightness[ring][position / divisor] = 255;
  }
  position += speed;

  if (position >= 5 * divisor) {
    --speed;
  } else {
    ++speed;
  }
  ++hue;

  return AnimationResult(__func__, 150);
}

static AnimationResult comets(SDL_Renderer *const renderer) {
  // These values don't matter, they'll be overwritten soon anyway, but I
  // wanted it to start at something other than all red
  static uint8_t spokeHue[SPOKE_COUNT] = {0,   20,  40,  60, 80,
                                          100, 120, 140, 160};
  static uint8_t spokeStart = 0;
  static uint8_t hue = 0;

  uint8_t r, g, b;

  for (int offset = 0; offset < RING_COUNT + 5; ++offset) {
    const int spoke = (spokeStart + offset) % SPOKE_COUNT;
    hsvToRgb(spokeHue[spoke], 255, 255, &r, &g, &b);
    setLed(RING_COUNT - offset - 1, spoke, r / 4, g / 4, b / 4, renderer);
    setLed(RING_COUNT - offset, spoke, r / 4, g / 4, b / 4, renderer);
    setLed(RING_COUNT - offset + 1, spoke, r / 3, g / 3, b / 3, renderer);
    setLed(RING_COUNT - offset + 2, spoke, r / 2, g / 2, b / 2, renderer);
    setLed(RING_COUNT - offset + 3, spoke, r / 3 * 2, g / 3 * 2, b / 3 * 2,
           renderer);
    setLed(RING_COUNT - offset + 4, spoke, r, g, b, renderer);
  }

  spokeHue[spokeStart] = hue;
  hue += 20;
  spokeStart = (spokeStart + 1) % SPOKE_COUNT;

  return AnimationResult(__func__, 400);
}

static AnimationResult cometsShort(SDL_Renderer *const renderer) {
  // These are going to be overridden soon anyway, so the exact values don't
  // matter, but let's make them not all start as red
  static uint8_t spokeHue[SPOKE_COUNT] = {0,   20,  40,  60, 80,
                                          100, 120, 140, 160};
  static uint8_t spokeStart = 0;
  static uint8_t hue = 0;

  uint8_t r, g, b;

  for (int offset = 0; offset < RING_COUNT + 2; ++offset) {
    const int spoke = (spokeStart + offset) % SPOKE_COUNT;
    hsvToRgb(spokeHue[spoke], 255, 255, &r, &g, &b);
    setLed(RING_COUNT - offset - 1, spoke, r / 4, g / 4, b / 4, renderer);
    setLed(RING_COUNT - offset, spoke, r / 2, g / 2, b / 2, renderer);
    setLed(RING_COUNT - offset + 1, spoke, r, g, b, renderer);
  }

  spokeHue[spokeStart] = hue;
  hue += 20;
  spokeStart = (spokeStart + 1) % SPOKE_COUNT;

  return AnimationResult(__func__, 400);
}

static AnimationResult snake(SDL_Renderer *const renderer) {
  static uint8_t hue = 0;
  static int index = 0;
  setLedHue(index, hue, renderer);
  ++hue;
  index = (index + 1) % (RING_COUNT * SPOKE_COUNT);
  return AnimationResult(__func__, 100);
}

static AnimationResult fadingRainbowRings(SDL_Renderer *const renderer) {
  //// red orange yellow green aqua blue purple
  // const uint8_t rainbowHues[] = {0, 22, 41, 80, 126, 165, 206};
  // red yellow green aqua-blue purple
  const uint8_t rainbowHues[] = {0, 41, 80, 145, 216};
  enum class Status {
    fadingIn,
    fadingOut,
  };
  const int change = 20;

  static int startHueIndex = 0;
  static int currentRing = 0;
  static uint8_t value = 40;
  static Status status = Status::fadingIn;

  uint8_t r, g, b;

  if (status == Status::fadingIn) {
    // Previous rings
    for (int ring = 0; ring < currentRing; ++ring) {
      const int hue =
          rainbowHues[(startHueIndex + ring) % COUNT_OF(rainbowHues)];
      hsvToRgb(hue, 255, 255, &r, &g, &b);
      for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
        setLed(ring, spoke, r, g, b, renderer);
      }
    }
    // Current ring
    const int hue =
        rainbowHues[(startHueIndex + currentRing) % COUNT_OF(rainbowHues)];
    hsvToRgb(hue, 255, value, &r, &g, &b);
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      setLed(currentRing, spoke, r, g, b, renderer);
    }
    if (currentRing == RING_COUNT - 1) {
      for (int spoke = SPOKE_COUNT; spoke < SPOKE_COUNT * 2; ++spoke) {
        setLed(currentRing, spoke, r, g, b, renderer);
      }
    }

    if (value < 255 - change) {
      value += change;
    } else {
      value = 0;
      ++currentRing;
      if (currentRing >= RING_COUNT) {
        currentRing = 0;
        status = Status::fadingOut;
        value = 250;
      }
    }
  } else {
    // Current ring
    const int hue =
        rainbowHues[(startHueIndex + currentRing) % COUNT_OF(rainbowHues)];
    hsvToRgb(hue, 255, value, &r, &g, &b);
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      setLed(currentRing, spoke, r, g, b, renderer);
    }
    if (currentRing == RING_COUNT - 1) {
      for (int spoke = SPOKE_COUNT; spoke < SPOKE_COUNT * 2; ++spoke) {
        setLed(currentRing, spoke, r, g, b, renderer);
      }
    }
    // Previous rings
    for (int ring = RING_COUNT - 1; ring > currentRing; --ring) {
      const int hue =
          rainbowHues[(startHueIndex + ring) % COUNT_OF(rainbowHues)];
      hsvToRgb(hue, 255, 255, &r, &g, &b);
      for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
        setLed(ring, spoke, r, g, b, renderer);
      }
      if (ring == RING_COUNT - 1) {
        for (int spoke = SPOKE_COUNT; spoke < SPOKE_COUNT * 2; ++spoke) {
          setLed(RING_COUNT - 1, spoke, r, g, b, renderer);
        }
      }
    }

    if (value > change) {
      value -= change;
    } else {
      value = 255;
      ++currentRing;
      if (currentRing >= RING_COUNT) {
        currentRing = 0;
        status = Status::fadingIn;
        value = 0;
        startHueIndex = (startHueIndex + 1) % COUNT_OF(rainbowHues);
      }
    }
  }

  return AnimationResult(__func__, 100);
}

int main() {
  bool shouldClose = false;
  uint8_t hue = 0;
  int animation_ms = 0;
  int animationIndex = 0;
  AnimationResult (*animations[])(SDL_Renderer *) = {pendulum,
                                                     orbit,
                                                     triadOrbits,
                                                     blurredSpiral,
                                                     blurredSpiralHues,
                                                     fadingRainbowRings,
                                                     cometsShort,
                                                     comets,
                                                     snake,
                                                     outwardRippleHue,
                                                     singleSpiral,
                                                     outwardRipple,
                                                     spiral,
                                                     lightAll,
                                                     spinSingle,
                                                     fastOutwardHue,
                                                     fastInwardHue};

  // Returns zero on success else non-zero
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    printf("error initializing SDL: %s\n", SDL_GetError());
  }
  SDL_Window *window = SDL_CreateWindow(
      "vest", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SDL_SetWindowTitle(window, animations[0](renderer).functionName);

  // Animation loop
  while (!shouldClose) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    const int radius = 160;
    filledCircleRGBA(renderer, WIDTH / 2 + LED_WIDTH / 2,
                     HEIGHT / 2 + LED_WIDTH / 2, radius, 255, 255, 255, 255);
    filledCircleRGBA(renderer, WIDTH / 2 + LED_WIDTH / 2,
                     HEIGHT / 2 + LED_WIDTH / 2, radius - 2, 0, 0, 0, 255);
    AnimationResult result = animations[animationIndex](renderer);
    ++hue;
    SDL_RenderPresent(renderer);

    while (result.delay_ms > 0) {
      // 60 FPS
      SDL_Delay(1000 / 60);
      result.delay_ms -= 60;

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
          name = animations[animationIndex](renderer).functionName;
          goto setTitle;

        previousAnimation:
          --animationIndex;
          if (animationIndex < 0) {
            animationIndex = COUNT_OF(animations) - 1;
          }
          name = animations[animationIndex](renderer).functionName;
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
  const float spacing = 20.0f;
  const float multiplier = (M_PI * 2.0 * (1.0 / SPOKE_COUNT));
  SDL_Rect rectangle;
  if (spoke >= 0 && spoke < SPOKE_COUNT) {
    if (ring >= 0 && ring < RING_COUNT) {
      const float angle_r = spoke * multiplier;
      const int xOffset = spacing * (ring + 3) * sinf(angle_r);
      const int yOffset = spacing * (ring + 3) * cosf(angle_r);
      // The ys are inverted
      rectangle.x = WIDTH / 2 + xOffset;
      rectangle.y = HEIGHT / 2 - yOffset;
      rectangle.w = LED_WIDTH;
      rectangle.h = LED_WIDTH;
      SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
      SDL_RenderFillRect(renderer, &rectangle);
    }
  } else if (ring == RING_COUNT - 1 && spoke >= SPOKE_COUNT &&
             spoke < SPOKE_COUNT * 2) {
    // The outer ring has extra spoke lights
    const float angle_r = (spoke - SPOKE_COUNT + 0.5) * multiplier;
    const int xOffset = spacing * (ring + 3) * sinf(angle_r);
    const int yOffset = spacing * (ring + 3) * cosf(angle_r);
    // The ys are inverted
    rectangle.x = WIDTH / 2 + xOffset;
    rectangle.y = HEIGHT / 2 - yOffset;
    rectangle.w = LED_WIDTH;
    rectangle.h = LED_WIDTH;
    SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
    SDL_RenderFillRect(renderer, &rectangle);
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
