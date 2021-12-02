#include <Adafruit_DotStar.h>
#include <cstdint>

#include "animations.hpp"

using std::uint8_t;
using std::uint32_t;

SingleColor::SingleColor() : color(0xFF0000) {
}

SingleColor::SingleColor(uint32_t color_) : color(color_) {
}

uint32_t SingleColor::getColor(Adafruit_DotStar&) {
  return color;
}

void SingleColor::nextFrame() {
}

UsaColors::UsaColors() : count(0) {
}

uint32_t UsaColors::getColor(Adafruit_DotStar&) {
  ++count;
  if (count <= 3) {
    // Red
    return 0xFF0000;
  }
  if (count <= 6) {
    // White
    return 0xFFFFFF;
  }
  // Blue
  return 0x0000FF;
}

void UsaColors::nextFrame() {
  count = 0;
}

RainbowColors::RainbowColors(uint16_t skipPerLed_, uint16_t skipPerIteration_) :
  hue(0),
  skipPerLed(skipPerLed_),
  skipPerIteration(skipPerIteration_),
  offset(hue)
{
}

uint32_t RainbowColors::getColor(Adafruit_DotStar& leds) {
  offset += skipPerLed;
  return leds.ColorHSV(offset, 255, 255);
}

void RainbowColors::nextFrame() {
  hue += skipPerIteration;
  offset = hue;
}

InchWormAnimation::InchWormAnimation(Adafruit_DotStar& leds_, uint8_t numLeds_, uint8_t length_, uint8_t start) :
  state(),
  leds(leds_),
  numLeds(numLeds_),
  length(length_),
  index(start < numLeds_ ? start : 0)
{
  if (index < length_) {
    state = inchWormState_t::BEGIN;
  } else if (index < numLeds_ - length) {
    state = inchWormState_t::MIDDLE;
  } else {
    state = inchWormState_t::END;
  }
}

void InchWormAnimation::animate(ColorFunctor& colorFunctor) {
  switch (state) {
    case inchWormState_t::BEGIN:
      for (int i = 0; i < index; ++i) {
        leds.setPixelColor(i, colorFunctor.getColor(leds));
      }
      ++index;
      // I don't ever expect index > length, but just to be defensive, do >=
      if (index >= length) {
        index = 0;
        state = inchWormState_t::MIDDLE;
      }
      break;
    case inchWormState_t::MIDDLE:
      for (int i = 0; i < length; ++i) {
        leds.setPixelColor(index + i, colorFunctor.getColor(leds));
      }
      ++index;
      // I don't ever expect index + length > INCH_WORM_LENGTH, but just to be defensive, do >=
      if (index + length >= numLeds) {
        index = 0;
        state = inchWormState_t::END;
      }
      break;
    case inchWormState_t::END:
      for (int i = 0; i < length - index; ++i) {
        leds.setPixelColor(numLeds - length + index + i, colorFunctor.getColor(leds));
      }
      ++index;
      // I don't ever expect index > length, but just to be defensive, do >=
      if (index >= length) {
        index = 0;
        state = inchWormState_t::BEGIN;
      }
      break;
    default:
      state = inchWormState_t::BEGIN;
      index = 0;
  }
}
