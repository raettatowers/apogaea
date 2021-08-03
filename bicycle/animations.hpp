#include <FastLED.h>

class ColorFunctor {
  public:
    virtual CRGB getColor() = 0;
    // Notifies the functor that the animation frame has completed, and it can prepare for the next frame
    virtual void reset() = 0;
};


class Animation {
  public:
    virtual void animate(ColorFunctor&) = 0;
};

#ifndef ANIMATIONS_HPP
#define ANIMATIONS_HPP

class SingleColor : public ColorFunctor {
  public:
    SingleColor();
    CRGB color;
    CRGB getColor();
    void reset();
};


class UsaColors : public ColorFunctor {
  private:
    uint8_t count;
  public:
    UsaColors();
    CRGB getColor();
    void reset();
};

class RainbowColors : public ColorFunctor {
  private:
    uint8_t hue;
    const uint8_t skipPerLed;
    const uint8_t skipPerIteration;
    uint8_t offset;
  public:
    RainbowColors(uint8_t skipPerLed_, uint8_t skipPerIteration_);
    CRGB getColor();
    void reset();
};

class InchWormAnimation : public Animation {
  public:
    InchWormAnimation(CRGB * leds_, uint8_t numLeds_, uint8_t length_, uint8_t start);
    void animate(ColorFunctor & colorFunctor);

  private:
    enum class inchWormState_t { BEGIN, MIDDLE, END };
    inchWormState_t state = inchWormState_t::BEGIN;
    CRGB * leds;
    const uint8_t numLeds;
    const uint8_t length;
    uint8_t index = 0;
};

class SpectrumAnalyzer : public Animation {
  public:
    SpectrumAnalyzer(CRGB * leds_, uint8_t numLeds_);
    void animate(ColorFunctor & colorFunctor);
  private:
    CRGB const * leds;
    const uint8_t numLeds;
};

#endif
