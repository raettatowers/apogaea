#include <FastLED.h>

class ColorFunctor {
  public:
    virtual CRGB getColor() = 0;
    virtual void reset() = 0;
};


class Animation {
  public:
    virtual void animate(ColorFunctor&) = 0;
};


class SingleColor : public ColorFunctor {
  public:
    SingleColor();
    CRGB color;
    CRGB getColor();
    void reset();
};


class UsaColors: public ColorFunctor {
  private:
    uint8_t count;
  public:
    UsaColors();
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
    CRGB const * leds;
    const uint8_t numLeds;
    const uint8_t length;
    uint8_t index = 0;
};
