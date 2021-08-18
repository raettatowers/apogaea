#ifndef ANIMATIONS_HPP
#define ANIMATIONS_HPP
#include <cstdint>

#include "constants.hpp"

class CRGB;


class Animation {
public:
  // Runs a tick of the animation, and returns the number of milliseconds until
  // the next time it should be called
  virtual int animate(const uint8_t hue) = 0;
  virtual ~Animation() = default;

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


class ShowBrightness : public Animation {
public:
  ShowBrightness();
  ~ShowBrightness() = default;
  int animate(uint8_t hue);
private:
  const int index;
};


class Ripple : public Animation {
public:
  Ripple();
  ~Ripple() = default;
  int animate(uint8_t hue);
private:
  int index;
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


class Shimmer : public Animation {
public:
  Shimmer();
  ~Shimmer() = default;
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

#endif