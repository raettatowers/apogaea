#include <cstdint>
#include <FastLED.h>

#include "animations.hpp"

extern CRGB leds[];

using std::uint8_t;

int lightAll() {
  static uint8_t hue = 0;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      setLedHue(ring, spoke, hue);
    }
  }
  ++hue;
  return 100;
}

int spinSingle() {
  static uint8_t hue = 0;
  static int spoke = 0;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    setLedHue(ring, spoke, hue);
  }
  ++hue;
  spoke = (spoke + 2) % SPOKE_COUNT;
  return 100;
}

int fastOutwardHue() {
  static uint8_t hue = 0;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      setLedHue(ring, spoke, hue - ring * 20);
    }
  }
  hue += 3;
  return 100;
}

int fastInwardHue() {
  static uint8_t hue = 0;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      setLedHue(ring, spoke, hue + ring * 20);
    }
  }
  hue += 3;
  return 100;
}

int spiral() {
  static uint8_t hue = 0;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      setLedHue(RING_COUNT - 1 - ring, SPOKE_COUNT - 1 - spoke,
                hue + ring * 20 + spoke * 10);
    }
  }
  hue += 3;
  return 100;
}

int outwardRipple() {
  static uint8_t hue = 0;
  static uint8_t ripple = 0;
  uint8_t r, g, b;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      hsvToRgb(hue + ring * 15, 255, sin8(ripple - ring * 30), &r, &g, &b);
      setLed(ring, spoke, r, g, b);
    }
  }
  ++hue;
  ripple += 3;
  return 100;
}

int outwardRippleHue() {
  static uint8_t hue = 0;
  static uint8_t ripple = 0;
  uint8_t r, g, b;
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      hsvToRgb(hue + ring * 15 + spoke * (255 / SPOKE_COUNT), 255,
               sin8(ripple - ring * 30), &r, &g, &b);
      setLed(ring, spoke, r, g, b);
    }
  }
  hue += 2;
  ripple += 3;
  return 100;
}

int singleSpiral() {
  static int spoke = 0;
  static uint8_t hue = 0;

  for (int ring = 0; ring < RING_COUNT; ++ring) {
    setLedHue(RING_COUNT - 1 - ring, (spoke + ring * 2) % SPOKE_COUNT, hue);
  }

  spoke = (spoke + 2) % SPOKE_COUNT;
  ++hue;

  return 400;
}

int blurredSpiral() {
  const int length = 5;
  const int brightnesses[] = {255 / 4, 255 / 2, 255, 255 / 2, 255 / 4};
  static_assert(COUNT_OF(brightnesses) == length);

  static int currentSpoke = 0;
  static uint8_t currentHue = 0;
  static int8_t starts[SPOKE_COUNT] = {0};

  uint8_t r, g, b;

  for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
    for (int offset = 0; offset < length; ++offset) {
      hsvToRgb(currentHue, 255, brightnesses[offset], &r, &g, &b);
      // Rely on the checks in setLed to not step out of bounds
      setLed(starts[spoke] + offset, spoke, r, g, b);
    }
    ++starts[spoke];
  }

  starts[currentSpoke] = -length + 1;
  currentSpoke = (currentSpoke + 1) % SPOKE_COUNT;
  ++currentHue;

  return 400;
}

int blurredSpiralHues() {
  const int length = 5;
  const int brightnesses[] = {255 / 4, 255 / 2, 255, 255 / 2, 255 / 4};
  static_assert(COUNT_OF(brightnesses) == length);

  static int currentSpoke = 0;
  static uint8_t currentHue = 0;
  static uint8_t hues[SPOKE_COUNT];
  static int8_t starts[SPOKE_COUNT] = {0};

  uint8_t r, g, b;

  for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
    for (int offset = 0; offset < length; ++offset) {
      hsvToRgb(hues[spoke], 255, brightnesses[offset], &r, &g, &b);
      // Rely on the checks in setLed to not step out of bounds
      setLed(starts[spoke] + offset, spoke, r, g, b);
    }
    ++starts[spoke];
  }

  starts[currentSpoke] = -length + 1;
  hues[currentSpoke] = currentHue;
  currentSpoke = (currentSpoke + 1) % SPOKE_COUNT;
  currentHue += 10;

  return 400;
}

int orbit() {
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
      setLed(ring, spoke, r, g, b);
    }
  }

  // The ball should always be maximum brightness
  setLedHue(position / divisor, currentSpoke, hue);
  brightness[position / divisor][currentSpoke] = 255;
  position += speed;

  if (position < 0) {
    position = 0;
    speed = startSpeed;
    currentSpoke = (currentSpoke + (SPOKE_COUNT / 2) + 1) % SPOKE_COUNT;
  }
  --speed;
  ++hue;

  return 150;
}

int triadOrbits() {
  const int startSpeed = 13;
  const int divisor = 16;
  const int fade = 10;

  static int position = -startSpeed;
  static int speed = startSpeed;
  static int currentSpoke = 0;
  static uint8_t hue = 0;
  static uint8_t brightness[RING_COUNT][SPOKE_COUNT] = {0};

  uint8_t r, g, b;
  // Draw all the fades
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
      setLed(ring, spoke, r, g, b);
    }
  }

  // The ball should always be maximum brightness
  for (int spoke = currentSpoke; spoke < SPOKE_COUNT; spoke += 6) {
    setLedHue(position / divisor, spoke, hue);
    brightness[position / divisor][spoke] = 255;
  }
  position += speed;

  if (position < 0) {
    position = 0;
    speed = startSpeed;
    currentSpoke = (currentSpoke + 2) % 6;
    hue += 50;
  }
  --speed;
  ++hue;

  return 150;
}

int pendulum() {
  const int divisor = 16;
  const int fade = 10;

  static int position = divisor * 2 + divisor / 3;
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
      setLed(ring, spoke, r, g, b);
    }
  }

  // The pendulum should always be maximum brightness
  for (int ring = 0; ring < RING_COUNT; ++ring) {
    setLedHue(ring, position / divisor, hue);
    brightness[ring][position / divisor] = 255;
  }
  position += speed;

  if (position >= 9 * divisor) {
    --speed;
  } else {
    ++speed;
  }
  ++hue;

  return 150;
}

int comets() {
  static uint8_t spokeHue[SPOKE_COUNT] = {0};
  static uint8_t spokeStart = 0;
  static uint8_t hue = 0;

  uint8_t r, g, b;

  for (int offset = 0; offset < RING_COUNT + 5; ++offset) {
    const int spoke = (spokeStart + offset) % SPOKE_COUNT;
    hsvToRgb(spokeHue[spoke], 255, 255, &r, &g, &b);
    setLed(RING_COUNT - offset - 1, spoke, r / 4, g / 4, b / 4);
    setLed(RING_COUNT - offset, spoke, r / 4, g / 4, b / 4);
    setLed(RING_COUNT - offset + 1, spoke, r / 3, g / 3, b / 3);
    setLed(RING_COUNT - offset + 2, spoke, r / 2, g / 2, b / 2);
    setLed(RING_COUNT - offset + 3, spoke, r / 3 * 2, g / 3 * 2, b / 3 * 2);
    setLed(RING_COUNT - offset + 4, spoke, r, g, b);
  }

  spokeHue[spokeStart] = hue;
  hue += 20;
  spokeStart = (spokeStart + 1) % SPOKE_COUNT;

  return 400;
}

int cometsShort() {
  static uint8_t spokeHue[SPOKE_COUNT] = {0};
  static uint8_t spokeStart = 0;
  static uint8_t hue = 0;

  uint8_t r, g, b;

  for (int offset = 0; offset < RING_COUNT + 2; ++offset) {
    const int spoke = (spokeStart + offset) % SPOKE_COUNT;
    hsvToRgb(spokeHue[spoke], 255, 255, &r, &g, &b);
    setLed(RING_COUNT - offset - 1, spoke, r / 4, g / 4, b / 4);
    setLed(RING_COUNT - offset, spoke, r / 2, g / 2, b / 2);
    setLed(RING_COUNT - offset + 1, spoke, r, g, b);
  }

  spokeHue[spokeStart] = hue;
  hue += 20;
  spokeStart = (spokeStart + 1) % SPOKE_COUNT;

  return 400;
}

int fadingRainbowRings() {
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
        setLed(ring, spoke, r, g, b);
      }
    }
    // Current ring
    const int hue =
        rainbowHues[(startHueIndex + currentRing) % COUNT_OF(rainbowHues)];
    hsvToRgb(hue, 255, value, &r, &g, &b);
    for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      setLed(currentRing, spoke, r, g, b);
    }
    if (currentRing == RING_COUNT - 1) {
      for (int spoke = SPOKE_COUNT; spoke < SPOKE_COUNT * 2; ++spoke) {
        setLed(currentRing, spoke, r, g, b);
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
      setLed(currentRing, spoke, r, g, b);
    }
    if (currentRing == RING_COUNT - 1) {
      for (int spoke = SPOKE_COUNT; spoke < SPOKE_COUNT * 2; ++spoke) {
        setLed(currentRing, spoke, r, g, b);
      }
    }
    // Previous rings
    for (int ring = RING_COUNT - 1; ring > currentRing; --ring) {
      const int hue =
          rainbowHues[(startHueIndex + ring) % COUNT_OF(rainbowHues)];
      hsvToRgb(hue, 255, 255, &r, &g, &b);
      for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
        setLed(ring, spoke, r, g, b);
      }
      if (ring == RING_COUNT - 1) {
        for (int spoke = SPOKE_COUNT; spoke < SPOKE_COUNT * 2; ++spoke) {
          setLed(RING_COUNT - 1, spoke, r, g, b);
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

  return 100;
}

int outerHue() {
  static uint8_t hue = 0;
  for (int spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
    setLedHue(RING_COUNT - 1, spoke, hue + (spoke * 255 / SPOKE_COUNT));
  }
  hue -= 10;

  return 100;
}

int outerRipple() {
  const int length = 7;
  const int brightnesses[] = {255 / 8, 255 / 4, 255 / 2, 255,
                                    255 / 2, 255 / 4, 255 / 8};
  static_assert(COUNT_OF(brightnesses) == length);

  static uint8_t hue = 0;
  static int spoke = 0;

  uint8_t r, g, b;

  for (int i = 0; i < length; ++i) {
    hsvToRgb(hue, 255, brightnesses[i], &r, &g, &b);
    setLed(RING_COUNT - 1, (spoke + i) % SPOKE_COUNT, r, g, b);
  }
  ++spoke;
  hue += 2;

  return 200;
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

static int ringSpokeToIndex(int ring, int spoke) {
  int value;
  if (ring < 0 || ring >= RING_COUNT || spoke < 0 || spoke >= SPOKE_COUNT) {
    return -1;
  }
  switch (spoke % 3) {
    case 0:
       value = (spoke / 3) * 11 + ring;
       if (value > LED_COUNT) {
         return -1;
       }
       return value;
    case 1:
      if (ring != RING_COUNT - 1) {
        return -1;
      }
       value = (spoke / 3) * 11 + 5;
       if (value > LED_COUNT) {
         return -1;
       }
       return value;
    case 2:
       value = (spoke / 3) * 11 + 6 + RING_COUNT - 1 - ring;
       if (value > LED_COUNT) {
         return -1;
       }
       return value;
    default:
      assert(false);
      return -1;
  }
}

void setLed(int ring, int spoke, uint8_t red, uint8_t green, uint8_t blue) {
  const int index = ringSpokeToIndex(ring, spoke);
  if (index > 0 && index < LED_COUNT) {
    leds[index] = CRGB(red, green, blue);
  }
}

void setLedHue(int ring, int spoke, uint8_t hue) {
  const int index = ringSpokeToIndex(ring, spoke);
  if (index > 0 && index < LED_COUNT) {
    leds[index] = CHSV(hue, 255, 255);
  }
}
