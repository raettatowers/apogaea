#include <FastLED.h>
#include <math.h>

#include "animations.hpp"
#include "constants.hpp"

CRGB leds[LED_COUNT];
CRGB dotStar[1];
const int LED_PIN = 0;
const int BUTTON_PIN = 3;


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
  dotStar[0] = CRGB::Blue;
  FastLED.show();
  delay(1000);
  FastLED.clear();
  FastLED.show();
}

int soundFunction() {
  return 0;
}

static uint8_t hue = 0;

static Snake snake(10, 2);
static HorizontalSnake horizontalSnake;
static Count count;
static CountXY countXY;
static Shine shine;
static Blobs blobs(3);
static Plasma rainbowPlasma(0.15f, 0.1f);
static Plasma pastelPlasma(0.15f, 0.1f, 1.0f, 1.0f, 0.0f, 0.0f, 3.0f / 2.0f * M_PI, 0.0f);
static SpectrumAnalyzer1 spectrumAnalyzer1(soundFunction);

static Animation* goodAnimations[] = { &snake, &blobs, &shine, &rainbowPlasma, nullptr };
static Animation* testAnimations[] = { &count, &countXY, &horizontalSnake, nullptr };
static Animation* snakeOnly[] = { &snake, nullptr };
static Animation* shineOnly[] = { &shine, nullptr };
static Animation* blobOnly[] = { &blobs, nullptr };
static Animation* plasmasOnly[] = { &rainbowPlasma, &pastelPlasma, nullptr };
static Animation** const animationSets[] = { goodAnimations, testAnimations, snakeOnly, plasmasOnly, shineOnly, blobOnly };
static uint8_t animationSetIndex = 0;
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

    const int delay_ms = animationSets[animationSetIndex][animationIndex]->animate(hue);
    FastLED.show();
    const auto start = millis();
    do {
      const int buttonState = digitalRead(BUTTON_PIN);
      if (buttonState == HIGH && previousButtonState != buttonState) {
        // Button pressed
        ++animationSetIndex;
        if (animationSetIndex > COUNT_OF(animationSets)) {
          animationSetIndex = 0;
        }
        animationIndex = 0;
        animationSets[0][0]->reset();
        animationStart_ms = millis();
        previousButtonState = buttonState;
        delay(20);  // Hacky debounce
        break;
      }
      previousButtonState = buttonState;
    } while (millis() < start + delay_ms);

    if (millis() > animationStart_ms + animationDuration_ms) {
      animationStart_ms = millis();
      ++animationIndex;
      if (animationSets[animationSetIndex][animationIndex] == nullptr) {
        // Some sets only have 1 animation; don't reset those
        if (animationIndex != 1) {
          animationSets[animationSetIndex][0]->reset();
        }
        animationIndex = 0;
      }
    }
  }
}
