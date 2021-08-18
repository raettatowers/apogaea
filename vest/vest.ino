#include <FastLED.h>

#include "animations.hpp"
#include "constants.hpp"

CRGB leds[LED_COUNT];
CRGB dotStar[1];
const int LED_PIN = 0;


void setup() {
  Serial.begin(9600);
  Serial.println("resetting");
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(&leds[0], COUNT_OF(leds)).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<DOTSTAR, 7, 8>(leds, 1);
  set_max_power_in_volts_and_milliamps(5, 800);
  FastLED.setBrightness(64);
  FastLED.clear();
  leds[5] = CRGB::Red;
  dotStar[0] = CRGB::Red;
  FastLED.show();
  delay(1000);
  FastLED.clear();
  FastLED.show();
}

int soundFunction() {
  return 0;
}

static uint8_t hue = 0;

Ripple ripple;
Snake snake;
ShowBrightness showBrightness;
Count count;
CountXY countXY;
Shimmer shimmer;
SpectrumAnalyzer1 spectrumAnalyzer1(soundFunction);
Animation* const animations[] = { &snake, &shimmer };
static uint8_t animationIndex = 0;


void loop() {
  const int hueDuration_ms = 50;
  const int animationDuration_ms = 10000;

  auto hueStart_ms = millis();
  auto animationStart_ms = millis();
  while (true) {
    if (millis() > hueStart_ms + hueDuration_ms) {
      hueStart_ms = millis();
      ++hue;
    }

    const int delay_ms = animations[animationIndex]->animate(hue);
    delay(delay_ms);
    if (millis() > animationStart_ms + animationDuration_ms) {
      animationStart_ms = millis();
      animationIndex = (animationIndex + 1) % COUNT_OF(animations);
    }

    FastLED.show();
  }
}
