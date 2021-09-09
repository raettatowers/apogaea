#include <FastLED.h>
#include <math.h>

#include "animations.hpp"
#include "constants.hpp"

CRGB leds[LED_COUNT];
CRGB dotStar[1];
const int LED_PIN = 0;
const int BUILT_IN_LED_PIN = 13;
const int BUTTON_PIN = 3;


void setup() {
  Serial.begin(9600);
  Serial.println("resetting");

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUILT_IN_LED_PIN, OUTPUT);
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

static HueGenerator hueGenerator;

static Snake snake(10, 2);
static HorizontalSnake horizontalSnake;
static Count count;
static CountXY countXY;
static Shine shine;
static Blobs blobs(3);
static PlasmaBidoulle plasmaBidoulleHue(0.15f, 0.1f);
static PlasmaBidoulle plasmaBidoullePastel(0.15f, 0.1f, 1.0f, 1.0f, 0.0f, 0.0f, 3.0f / 2.0f * M_PI, 0.0f);
static PlasmaBidoulleFast plasmaBidoulleFastHue(hueGenerator);
static Plasma1 plasma1Hue(hueGenerator);
static Plasma2 plasma2Hue(hueGenerator);
static Plasma2 plasma3Hue(hueGenerator);
static SpectrumAnalyzer1 spectrumAnalyzer1(soundFunction);

static Animation* goodAnimations[] = { &plasmaBidoulleFastHue, &plasma1Hue, &snake, &plasma2Hue, &blobs, &plasma3Hue, &shine, &plasmaBidoullePastel, nullptr };
static Animation* testAnimations[] = { &count, &countXY, &horizontalSnake, nullptr };
static Animation** const animationSets[] = { goodAnimations, testAnimations };
static uint8_t animationSetIndex = 0;
static uint8_t animationIndex = 0;
bool cycling = true;
unsigned long animationStart_ms = millis();

void loop() {
  const int hueDuration_ms = 50;
  const int animationDuration_ms = 10000;

  unsigned long hueStart_ms = millis();
  while (true) {
    if (millis() > hueStart_ms + hueDuration_ms) {
      hueStart_ms = millis();
      ++hue;
    }

    const int delay_ms = animationSets[animationSetIndex][animationIndex]->animate(hue);
    FastLED.show();
    delayAndCheckForButton(delay_ms);

    if (cycling && millis() > animationStart_ms + animationDuration_ms) {
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


void delayAndCheckForButton(const int delay_ms) {
  const unsigned long start = millis();
  const int buttonState = digitalRead(BUTTON_PIN);
  do {
    // Button pressed
    if (buttonState == LOW) {
      if (cycling) {
        animationIndex = 0;
        ++animationSetIndex;
        if (animationSetIndex == COUNT_OF(animationSets)) {
          animationSetIndex = 0;
          animationSets[0][0]->reset();
          cycling = !cycling;
        }
      } else {
        ++animationIndex;
        if (animationSets[animationSetIndex][animationIndex] == nullptr) {
          animationIndex = 0;
          ++animationSetIndex;
          if (animationSetIndex == COUNT_OF(animationSets)) {
            animationSetIndex = 0;
            animationSets[0][0]->reset();
            cycling = !cycling;
          }
        }
      }

      // Wait for the button to stop being pressed
      delay(20); // Hacky debounce
      while (digitalRead(BUTTON_PIN) == LOW) {}
      animationStart_ms = millis();
      return;
    }

  } while (millis() < start + delay_ms);
}
