#ifndef ANIMATIONS_HPP
#define ANIMATIONS_HPP

#include <cstdint>
#include <arduinoFFT.h>
#include <Adafruit_DotStar.h>

#include "constants.hpp"

using std::uint8_t;

class ColorFunctor {
  public:
    virtual uint32_t getColor(Adafruit_DotStar& leds) = 0;
    // Notifies the functor that the animation frame has completed, and it can prepare for the next frame
    virtual void nextFrame() = 0;
};

class Animation {
  public:
    virtual void animate(ColorFunctor&) = 0;
};

class SingleColor : public ColorFunctor {
  public:
    SingleColor();
    SingleColor(uint32_t color);
    uint32_t color;
    uint32_t getColor(Adafruit_DotStar& leds);
    void nextFrame();
};


class UsaColors : public ColorFunctor {
  public:
    UsaColors();
    uint32_t getColor(Adafruit_DotStar& leds);
    void nextFrame();
  private:
    uint8_t count;
};

class RainbowColors : public ColorFunctor {
  public:
    RainbowColors(uint16_t skipPerLed_, uint16_t skipPerIteration_);
    uint32_t getColor(Adafruit_DotStar& leds);
    void nextFrame();
  private:
    uint16_t hue;
    const uint16_t skipPerLed;
    const uint16_t skipPerIteration;
    uint16_t offset;
};

class InchWormAnimation : public Animation {
  public:
    InchWormAnimation(Adafruit_DotStar& strip, uint8_t numLeds, uint8_t length, uint8_t start);
    void animate(ColorFunctor& colorFunctor);

  private:
    enum class inchWormState_t {BEGIN, MIDDLE, END};
    inchWormState_t state = inchWormState_t::BEGIN;
    Adafruit_DotStar& leds;
    const uint8_t numLeds;
    const uint8_t length;
    uint8_t index = 0;
};

class SpectrumAnalyzer : public Animation {
  public:
    SpectrumAnalyzer(Adafruit_DotStar& leds, uint8_t numLeds);
    void animate(ColorFunctor& colorFunctor);
  private:
    Adafruit_DotStar& leds;
    const int numLeds;
    arduinoFFT fft;
    uint8_t previousValues[LED_COUNT];
};

#endif
