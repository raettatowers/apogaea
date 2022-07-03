#include <FastLED.h>
#include "animations.hpp"
#include "constants.hpp"

// This include is needed to fix "undefined reference to `Adafruit_USBD_CDC::begin(unsigned long)'"
//#include <Adafruit_CircuitPlayground.h>

// Just a single strand running up the top tube

// How many leds in your strip?
const int NUM_LEDS = 100;

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
const int DATA_PIN = 2;
const int BUTTON_PIN = 11;

// Define the array of leds
CRGB leds[NUM_LEDS];

Streak streak1(leds, NUM_LEDS, 9, 0);
Streak streak2(leds, NUM_LEDS, 9, NUM_LEDS / 2);
Animation* pair[] = {&streak1, &streak2};
MultipleAnimations streaks(leds, NUM_LEDS, pair, COUNT_OF(pair));
Comet comet(leds, NUM_LEDS);

UsaColors usaColors;
RainbowColors rainbowColors(1, 1);
RainbowColors iteratingColors1(20, 20), iteratingColors2(20, 20);
SingleColor singleColor;

Animation* animations[] = {
  &streaks,
  &comet,
};

ColorFunctor* colorFunctors[] = {
  //&singleColor,
  //&usaColors,
  &rainbowColors,
  &iteratingColors1,
};


void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, BGR>(leds, NUM_LEDS);
  FastLED.setBrightness(64);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 400);
  FastLED.clear();
  pinMode(DATA_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  singleColor.color = CRGB::Green;
  Serial.begin(115200);
}


const int maxSleepTime_ms = 150;
const int minSleepTime_ms = 30;
const int sleepTimeIncrement_ms = 3;
int sleepTime_ms = minSleepTime_ms;
int sleepTimeStep_ms = -sleepTimeIncrement_ms;
uint8_t animationIndex = 0;
uint8_t colorFunctorIndex = 0;

auto start_ms = millis();

void loop() {
  // Do animations here
  ColorFunctor* functor = colorFunctors[colorFunctorIndex];
  animations[animationIndex]->animate(*functor);
  functor->reset();

  FastLED.show();
  // Vary the speed of the animation
  sleepTime_ms += sleepTimeStep_ms;
  if (sleepTime_ms >= maxSleepTime_ms) {
    sleepTimeStep_ms = -sleepTimeIncrement_ms;
  } else if (sleepTime_ms <= minSleepTime_ms) {
    sleepTimeStep_ms = sleepTimeIncrement_ms;
  }
  auto start = millis();
  while (millis() < start + sleepTime_ms) {
    //checkAndHandleButton();
  }
  FastLED.clear();

  if (start_ms + 30000 < millis()) {
    ++animationIndex;
    if (animationIndex == COUNT_OF(animations)) {
      animationIndex = 0;
      colorFunctorIndex = (colorFunctorIndex + 1) % COUNT_OF(colorFunctors);
    }
    start_ms = millis();
  }
}


void checkAndHandleButton() {
  return;
  static int debounceTime_ms = 0;
  static bool pressed = false;

  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == HIGH) {
    // Not pressed
    pressed = false;
    digitalWrite(LED_BUILTIN, LOW);
    return;
  }

  if (debounceTime_ms - millis() < 25) {
    // Just a bounce, ignore it
    return;
  }

  debounceTime_ms = millis();
  // Avoid continual presses
  if (!pressed) {
    pressed = true;
    digitalWrite(LED_BUILTIN, HIGH);
    colorFunctorIndex = (colorFunctorIndex + 1) % COUNT_OF(colorFunctors);
  }
}

// Fix for "undefined reference to `std::__throw_bad_alloc()'"
namespace std {
  void __throw_bad_alloc()
  {
    Serial.println("Unable to allocate memory");
    for (;;);
  }
}
