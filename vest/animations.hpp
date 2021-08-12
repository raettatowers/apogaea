#ifndef ANIMATIONS_HPP
#define ANIMATIONS_HPP
#include <cstdint>


class Animation {
public:
  // Runs a tick of the animation, and returns the number of milliseconds until
  // the next time it should be called
  virtual int animate(uint32_t color) = 0;
  virtual ~Animation() = default;

  static void setLed(int x, int y, uint32_t color);
  static void setLed(int index, uint32_t color);
};


class Count : public Animation {
public:
  Count();
  ~Count() = default;
  int animate(uint32_t color);
private:
  int index;
};


class Snake : public Animation {
public:
  Snake();
  ~Snake() = default;
  int animate(uint32_t color);
private:
  int index;
};


class ShowBrightness : public Animation {
public:
  ShowBrightness();
  ~ShowBrightness() = default;
  int animate(uint32_t color);
private:
  int index;
};


class Ripple : public Animation {
public:
  Ripple();
  ~Ripple() = default;
  int animate(uint32_t color);
private:
  int index;
};

#endif
