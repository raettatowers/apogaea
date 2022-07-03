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
    uint8_t value;
  public:
    RainbowColors(uint8_t skipPerLed_, uint8_t skipPerIteration_);
    CRGB getColor();
    void reset();
};

class MultipleAnimations : public Animation {
  public:
    MultipleAnimations(
      CRGB* leds,
      uint8_t ledCount,
      Animation** animations,
      uint8_t animationCount
    );
    ~MultipleAnimations() = default;
    void animate(ColorFunctor& colorFunctor) override;
  private:
    CRGB* const leds;
    const uint8_t ledCount;
    Animation** animations;
    const uint8_t animationCount;

    MultipleAnimations(MultipleAnimations&) = delete;
    MultipleAnimations(MultipleAnimations&&) = delete;
};

class Streak : public Animation {
  public:
    Streak(CRGB * leds_, uint8_t numLeds_, uint8_t length_, uint8_t start);
    void animate(ColorFunctor & colorFunctor);

  protected:
    enum class streakState_t { BEGIN, MIDDLE, END };
    streakState_t state;
    CRGB * const leds;
    const uint8_t numLeds;
    const uint8_t length;
    uint8_t index = 0;

    Streak(Streak&) = delete;
    Streak(Streak&&) = delete;
};

class Comet : public Animation {
  public:
    Comet(CRGB* leds, uint8_t ledCount);
    void animate(ColorFunctor&) override;

  private:
    CRGB* const leds;
    const uint8_t ledCount;
    uint8_t start;
    uint8_t hue;
    uint8_t* hues;
    uint8_t* brightnesses;

    Comet(Comet&) = delete;
    Comet(Comet&&) = delete;
};
