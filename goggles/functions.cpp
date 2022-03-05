#include <FastLED.h>

CRGB gbChsv(const uint8_t hue, const uint8_t, const uint8_t) {
  // Normally red = sin8(hue), green = sin8(hue + 256 / 3), blue = sin8(hue + 256 * 2 / 3)
  // But I'm disabling red, so it should be green = sin8(hue), blue = sin8(hue + 256 / 2)
  return CRGB(0, sin8(hue), sin8(hue + 128));
}
