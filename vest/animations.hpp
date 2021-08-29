#ifndef ANIMATIONS_HPP
#define ANIMATIONS_HPP
#include <stdint.h>

#include "constants.hpp"

class CRGB;


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
    Snake();
    ~Snake() = default;
    int animate(uint8_t hue);
  private:
    int startIndex;
    int endIndex;
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


class Plasma : public Animation {
  public:
    // 0.15, 0.1 works well
    Plasma(float multiplier, float timeIncrement);
    ~Plasma() = default;
    int animate(uint8_t hue);
  private:
    float time;
    const float multiplier;
    const float timeIncrement;

    static uint8_t convert(const float f) {
      return static_cast<uint8_t>((f + 1.0f) * 0.5f * 255.0f);
    }
};

#endif
