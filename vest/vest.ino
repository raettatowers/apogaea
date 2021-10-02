#include <FastLED.h>
#include <math.h>

#include "animations.hpp"
#include "constants.hpp"
#include "video/rick_roll_centered.hpp"

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

static uint8_t hue = 0;

static int soundFunction() {
  return 0;
}

static ColorGenerator hueGenerator;
static RedGreenGenerator redGreenGenerator;
static PastelGenerator pastelGenerator;
static NeonGenerator neonGenerator;
static ChangingGenerator changingGenerator;

static CenteredVideo rickRoll(
  [](int a, int b) { return RICK_ROLL_CENTERED[a][b]; },
  COUNT_OF(RICK_ROLL_CENTERED),
  RICK_ROLL_CENTERED_MILLIS_PER_FRAME
);
static Snake snake(10, 2);
static HorizontalSnake horizontalSnake;
static Count count;
static CountXY countXY;
static Shine shine;
static Blobs blobs(4);
static PlasmaBidoulleFast plasma1(neonGenerator);
static PlasmaBidoulleFast plasma2(redGreenGenerator);
static PlasmaBidoulleFast plasma3(pastelGenerator);
static Plasma3 plasma4(hueGenerator);
static PlasmaBidoulleFast plasma5(changingGenerator);
static BasicSpiral spiral(hueGenerator);
static SpectrumAnalyzer1 spectrumAnalyzer1(soundFunction);

static constexpr Animation* goodAnimations[] = { &spiral, &plasma5, &plasma1, &snake, &plasma2, &blobs, &plasma3, &shine, &plasma4, nullptr };
static_assert(goodAnimations[COUNT_OF(goodAnimations) - 1] == nullptr);
static constexpr Animation* testAnimations[] = { &count, &countXY, &horizontalSnake, nullptr };
static_assert(testAnimations[COUNT_OF(testAnimations) - 1] == nullptr);
static constexpr Animation* videoAnimations[] = { &rickRoll, nullptr };
static_assert(videoAnimations[COUNT_OF(videoAnimations) - 1] == nullptr);
static constexpr Animation*const* animationSets[] = { goodAnimations, videoAnimations, testAnimations };
static uint8_t animationSetIndex = 0;
static uint8_t animationIndex = 0;
static bool cycling = true;
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
