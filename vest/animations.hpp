#ifndef ANIMATIONS_HPP
#define ANIMATIONS_HPP

class CRGB;

class Animation {
public:
  virtual void animate(const CRGB& color) = 0; 
};


class Count : public Animation {
public:
  void animate(const CRGB& color);
  Count(CRGB& leds, int ledCount);
private:
  CRGB& leds;
  const int ledCount;
  int index;
  unsigned long previousMillis;
};


class Snake : public Animation {
public:
  void animate(const CRGB& color);
  Snake(CRGB& leds, int ledCount);
private:
  CRGB& leds;
  const int ledCount;
  int index;
  unsigned long previousMillis;
};


class ShowBrightness : public Animation {
public:
  void animate(const CRGB& color);
  ShowBrightness(CRGB& leds, int ledCount);
private:
  CRGB& leds;
  const int ledCount;
  int index;
  unsigned long previousMillis;
};

#endif
