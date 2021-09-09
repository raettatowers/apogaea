#ifndef ANIMATIONS_HPP
#define ANIMATIONS_HPP
#include <FastLED.h>
#include <stdint.h>

#include "constants.hpp"

class CRGB;

class ColorGenerator {
  public:
    virtual CHSV getColor(uint8_t v) = 0;
};

class Animation {
  public:
    // Runs a tick of the animation, and returns the number of milliseconds until
    // the next time it should be called
    virtual int animate(const uint8_t hue) = 0;
    virtual ~Animation() = default;
    virtual void reset() {
    }

    static void setLed(int x, int y, const CRGB& color);
    static void setLed(int index, const CRGB& color);
};

class Count : public Animation {
  public:
    Count();
    ~Count() = default;
    int animate(uint8_t hue);
  private:
    int index;
};

class CountXY : public Animation {
  public:
    CountXY();
    ~CountXY() = default;
    int animate(uint8_t hue);
  private:
    int index;
};

class Snake : public Animation {
  public:
    Snake(int length, int count);
    ~Snake();
    int animate(uint8_t hue);
  private:
    const int length;
    const int count;
    int* startIndexes;
    int* endIndexes;
};

class HorizontalSnake : public Animation {
  public:
    HorizontalSnake();
    ~HorizontalSnake() = default;
    int animate(uint8_t hue);
  private:
    int x;
    int y;
    bool xIncreasing;
};

class Fire : public Animation {
  public:
    Fire();
    ~Fire() = default;
    int animate(uint8_t hue);
  private:
    uint32_t colors[LED_COUNT];
    uint8_t heights[10];
};

class Shine : public Animation {
  public:
    Shine();
    ~Shine() = default;
    int animate(uint8_t hue);
  private:
    bool increasing[LED_COUNT];
    int amount[LED_COUNT];
    uint8_t hues[LED_COUNT];
};

class SpectrumAnalyzer1 : public Animation {
  public:
    SpectrumAnalyzer1(int (*soundFunction)(void));
    ~SpectrumAnalyzer1() = default;
    int animate(uint8_t hue);
  private:
    int (*soundFunction)(void);
};

class Blobs : public Animation {
  public:
    Blobs(int count_);
    ~Blobs();
    int animate(uint8_t hue);
  private:
    const int count;
    float* targetX;
    float* targetY;
    float* x;
    float* y;
    float* xSpeed;
    float* ySpeed;
};

class PlasmaBidoulle : public Animation {
  public:
    // 0.15, 0.1 works well
    PlasmaBidoulle(float multiplier, float timeIncrement);
    PlasmaBidoulle(
      float multiplier,
      float timeIncrement,
      float redMultiplier,
      float greenMultiplier,
      float blueMultiplier,
      float redOffset,
      float greenOffset,
      float blueOffset
    );
    ~PlasmaBidoulle() = default;
    int animate(uint8_t hue);
  private:
    float time;
    const float multiplier;
    const float timeIncrement;
    const float redMultiplier;
    const float greenMultiplier;
    const float blueMultiplier;
    const float redOffset;
    const float greenOffset;
    const float blueOffset;

    static uint8_t convert(const float f);
};


// A (partial) implementation of Bidoulle's animation, using fast 16-bit math
class PlasmaBidoulleFast : public Animation {
  public:
    PlasmaBidoulleFast(ColorGenerator& colorGenerator);
    ~PlasmaBidoulleFast() = default;
    int animate(uint8_t hue);
  private:
    ColorGenerator& colorGenerator;
    uint32_t time;
};

class Plasma1 : public Animation {
  public:
    Plasma1(ColorGenerator& colorGenerator);
    ~Plasma1() = default;
    int animate(uint8_t hue);
  private:
    ColorGenerator& colorGenerator;
    int time;
};

class Plasma2 : public Animation {
  public:
    Plasma2(ColorGenerator& colorGenerator);
    ~Plasma2() = default;
    int animate(uint8_t hue);
  private:
    ColorGenerator& colorGenerator;
    int time;
};

class Plasma3 : public Animation {
  public:
    Plasma3(ColorGenerator& colorGenerator);
    ~Plasma3() = default;
    int animate(uint8_t hue);
  private:
    ColorGenerator& colorGenerator;
    int time;
};

class HueGenerator : public ColorGenerator {
  public:
    HueGenerator() = default;
    ~HueGenerator() = default;
    HueGenerator(const HueGenerator&) = delete;

    CHSV getColor(uint8_t value) {
      return CHSV(value, 255, 255);
    }
};

#endif
