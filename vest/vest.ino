#include <FastLED.h>
#include <math.h>
// Needed to get Serial to work
#include "Adafruit_TinyUSB.h"

#include "animations.hpp"
#include "constants.hpp"

CRGB leds[LED_COUNT];
//CRGB dotStar[1];
// 15 = A1 / D6
const int LED_PIN = 15;
//const int BUILT_IN_LED_PIN = 13;

void blinkTimes(const int count) {
  if (count == 0) {
    for (int i = 0; i < 3; ++i) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(75);
      digitalWrite(LED_BUILTIN, LOW);
      delay(75);
    }
  } else {
    for (int i = 0; i < count; ++i) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(250);
      digitalWrite(LED_BUILTIN, LOW);
      delay(250);
    }
  }
  delay(1000);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  //pinMode(BUILT_IN_LED_PIN, OUTPUT);
  Serial.begin(9600);

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(&leds[0], COUNT_OF(leds)).setCorrection(TypicalLEDStrip);
  //set_max_power_in_volts_and_milliamps(5, 300);
  FastLED.setBrightness(64);
  FastLED.clear();
  leds[1] = CRGB::Red;
  leds[2] = CRGB::Blue;
  leds[3] = CRGB::Green;
  FastLED.clear();
  FastLED.show();
  blinkTimes(3);
}

static uint8_t hue = 0;

static int soundFunction() {
  return 0;
}

static ColorGenerator hueGenerator;
static RedGreenGenerator redGreenGenerator;
static PastelGenerator pastelGenerator;
static NeonGenerator neonGenerator;
static ChangingGenerator changingGenerator;
static ChristmasGenerator christmasGenerator;

static Snake snake(10, 2);
static HorizontalSnake horizontalSnake;
static Count count;
static CountXY countXY;
static Shine shine;
static Blobs blobs(4);
static PlasmaBidoulleFast plasmaChristmas(christmasGenerator);
static PlasmaBidoulleFast plasma1(neonGenerator);
static PlasmaBidoulleFast plasma2(redGreenGenerator);
static PlasmaBidoulleFast plasma3(pastelGenerator);
static Plasma3 plasma4(hueGenerator);
static PlasmaBidoulleFast plasma5(changingGenerator);
static BasicSpiral spiral(hueGenerator);
static SpectrumAnalyzer1 spectrumAnalyzer1(soundFunction);

static constexpr Animation* goodAnimations[] = { &plasmaChristmas, &spiral, &plasma5, &plasma1, &snake, &plasma2, &blobs, &plasma3, &shine, &plasma4, nullptr };
static_assert(goodAnimations[COUNT_OF(goodAnimations) - 1] == nullptr);
static constexpr Animation* testAnimations[] = { &count, &countXY, &horizontalSnake, nullptr };
static_assert(testAnimations[COUNT_OF(testAnimations) - 1] == nullptr);
static constexpr Animation* snakeOnly[] = { &snake, nullptr };
static_assert(snakeOnly[COUNT_OF(snakeOnly) - 1] == nullptr);
static constexpr Animation* videoAnimations[] = { nullptr };
static_assert(videoAnimations[COUNT_OF(videoAnimations) - 1] == nullptr);
//static constexpr Animation* const* animationSets[] = { goodAnimations, testAnimations };
static constexpr Animation* const* animationSets[] = { snakeOnly };
static uint8_t animationSetIndex = 0;
static uint8_t animationIndex = 0;
static bool cycling = false;
static unsigned long animationStart_ms = millis();

void loop() {
  const int hueDuration_ms = 50;
  const int animationDuration_ms = 30000;

  unsigned long hueStart_ms = millis();
  while (true) {
    if (millis() > hueStart_ms + hueDuration_ms) {
      hueStart_ms = millis();
      ++hue;
    }

    Serial.print(static_cast<int>(animationSetIndex));
    Serial.print(" ");
    Serial.println(static_cast<int>(animationIndex));
    blinkTimes(animationSetIndex);
    blinkTimes(animationIndex);
    const int delay_ms = animationSets[animationSetIndex][animationIndex]->animate(hue);
    blinkTimes(1);
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


static bool buttonPressed() {
  return false;
  // return digitalRead(BUTTON_PIN) == LOW;
  // return CircuitPlayground.leftButton() || CircuitPlayground.rightButton();
}


static void delayAndCheckForButton(const int delay_ms) {
  const unsigned long start = millis();
  const bool buttonState = buttonPressed();
  do {
    // Button pressed
    if (buttonState) {
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
      while (buttonPressed()) {}
      animationStart_ms = millis();
      return;
    }

  } while (millis() < start + delay_ms);
}
