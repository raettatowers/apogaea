#ifndef DEMO
#include <FastLED.h>
#endif

class LedArray {
  public:
  #ifdef DEMO
  #else
    LedArray(CRGB* array, int size);
  #endif
  
  private:
  #ifdef DEMO
  #else
    CRGB* const array;
    const int size;
  #endif
};
