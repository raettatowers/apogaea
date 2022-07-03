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


RainbowColors::RainbowColors(uint8_t skipPerLed_, uint8_t skipPerIteration_) :
  hue(0),
  skipPerLed(skipPerLed_),
  skipPerIteration(skipPerIteration_),
  offset(hue),
  value(50)
{
}


CRGB RainbowColors::getColor() {
  offset += skipPerLed;
  if (value < 255 - 20) {
    value += 20;
  } else {
    value = 255;
  }
  return CHSV(offset, 255, value);
}


void RainbowColors::reset() {
  hue += skipPerIteration;
  offset = hue;
  value = 0;
}

MultipleAnimations::MultipleAnimations(
  CRGB* const leds_,
  const uint8_t ledCount_,
  Animation** animations_,
  const uint8_t animationCount_
) :
  leds(leds_),
  ledCount(ledCount_),
  animations(animations_),
  animationCount(animationCount_)
{
}

void MultipleAnimations::animate(ColorFunctor& functor) {
  for (int i = 0; i < animationCount; ++i) {
    animations[i]->animate(functor);
  }
}

Streak::Streak(CRGB * leds_, uint8_t numLeds_, uint8_t length_, uint8_t start) :
  state(streakState_t::BEGIN),
  leds(leds_),
  numLeds(numLeds_),
  length(length_),
  index(start < numLeds_ ? start : 0)
{
  if (index < length_) {
    state = streakState_t::BEGIN;
  } else if (index < numLeds_ - length) {
    state = streakState_t::MIDDLE;
  } else {
    state = streakState_t::END;
  }
}

void Streak::animate(ColorFunctor & colorFunctor) {
  switch (state) {
    case streakState_t::BEGIN:
      for (int i = 0; i < index; ++i) {
        leds[i] = colorFunctor.getColor();
      }
      ++index;
      // I don't ever expect index > length, but just to be defensive, do >=
      if (index >= length) {
        index = 0;
        state = streakState_t::MIDDLE;
      }
      break;
    case streakState_t::MIDDLE:
      for (int i = 0; i < length; ++i) {
        leds[index + i] = colorFunctor.getColor();
      }
      ++index;
      // I don't ever expect index + length > INCH_WORM_LENGTH, but just to be defensive, do >=
      if (index + length >= numLeds) {
        index = 0;
        state = streakState_t::END;
      }
      break;
    case streakState_t::END:
      for (int i = 0; i < length - index; ++i) {
        leds[numLeds - length + index + i] = colorFunctor.getColor();
      }
      ++index;
      // I don't ever expect index > length, but just to be defensive, do >=
      if (index >= length) {
        index = 0;
        state = streakState_t::BEGIN;
      }
      break;
    default:
      state = streakState_t::BEGIN;
      index = 0;
  }
}

Comet::Comet(CRGB* leds_, uint8_t ledCount_) :
  leds(leds_),
  ledCount(ledCount_),
  start(0),
  hue(0),
  hues(new uint8_t[ledCount_]),
  brightnesses(new uint8_t[ledCount_])
{
  memset(hues, 0, ledCount_ * sizeof(hues[0]));
  memset(brightnesses, 0, ledCount_ * sizeof(brightnesses[0]));
}

void Comet::animate(ColorFunctor&) {
  const int offBrightness = 0;
  const int brightnessStep = 20;
  const int cometBrightness = 255;
  const int length = 10;

  fill_solid(leds, ledCount, CHSV(hue + 128, 255, offBrightness));
  uint8_t brightness = 255 - length * brightnessStep;
  static_assert(cometBrightness - brightnessStep * length > 0);
  for (int i = 0; i < length; ++i) {
    leds[(start + i) % ledCount] = CHSV(hue, 255, brightness);
    brightness += brightnessStep;
  }
  ++hue;
  ++start;
}
