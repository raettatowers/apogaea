#include <FastLED.h>
#include "animations.hpp"

SingleColor::SingleColor() : color(CRGB::Red) {
}


CRGB SingleColor::getColor() {
  return color;
}


void SingleColor::reset() {
}


UsaColors::UsaColors() : count(0) {
}


CRGB UsaColors::getColor() {
  ++count;
  if (count <= 3) {
    return CRGB::Red;
  }
  if (count <= 6) {
    return CRGB::White;
  }
  return CRGB::Blue;
}


void UsaColors::reset() {
  count = 0;
}


InchWormAnimation::InchWormAnimation(CRGB * leds_, uint8_t numLeds_, uint8_t length_, uint8_t start) :
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


void InchWormAnimation::animate(ColorFunctor & colorFunctor) {
  switch (state) {
    case inchWormState_t::BEGIN:
      for (int i = 0; i < index; ++i) {
        leds[i] = colorFunctor.getColor();
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
        leds[index + i] = colorFunctor.getColor();
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
        leds[numLeds - length + index + i] = colorFunctor.getColor();
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
