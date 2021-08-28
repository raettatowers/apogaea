#include <FastLED.h>

#include "animations.hpp"
#include "constants.hpp"

CRGB leds[LED_COUNT];
CRGB dotStar[1];
const int LED_PIN = 0;
const int BUTTON_PIN = 3;

#define USE_BUTTON 1


void setup() {
  Serial.begin(9600);
  Serial.println("resetting");

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

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

Snake snake;
HorizontalSnake horizontalSnake;
Count count;
CountXY countXY;
Shine shine;
Blobs blobs(3);
SpectrumAnalyzer1 spectrumAnalyzer1(soundFunction);
//Animation* const animations[] = { &count, &countXY, &horizontalSnake };
Animation* const animations[] = { &snake, &blobs, &shine };
static uint8_t animationIndex = 0;


void loop() {
  const int hueDuration_ms = 50;
  const int animationDuration_ms = 10000;
  int previousButtonState = digitalRead(BUTTON_PIN);

  auto hueStart_ms = millis();
  auto animationStart_ms = millis();
  while (true) {
    if (millis() > hueStart_ms + hueDuration_ms) {
      hueStart_ms = millis();
      ++hue;
    }

    const int delay_ms = animations[animationIndex]->animate(hue);
    const auto start = millis();
    while (millis() < start + delay_ms) {
      const int buttonState = digitalRead(BUTTON_PIN);
#if USE_BUTTON
      if (buttonState == HIGH && previousButtonState != buttonState) {
#else
      if (millis() > animationStart_ms + animationDuration_ms) {
#endif
        ++animationIndex;
        if (animationIndex == COUNT_OF(animations)) {
          animationIndex = 0;
        }
        animations[animationIndex]->reset();
        previousButtonState = buttonState;
        delay(20);  // Debounce
        break;
      }
      previousButtonState = buttonState;
    }

    FastLED.show();
  }
}
