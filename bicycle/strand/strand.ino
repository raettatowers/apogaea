#include <FastLED.h>
#include "animations.hpp"

// Just a single strand running up the top tube

// How many leds in your strip?
#define NUM_LEDS 57

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
const int DATA_PIN = 7;
const int CLOCK_PIN = 6;
const int BUTTON_PIN = 11;

// Define the array of leds
CRGB leds[NUM_LEDS];

InchWormAnimation worm1(leds, NUM_LEDS, 9, 0);
InchWormAnimation worm2(leds, NUM_LEDS, 9, NUM_LEDS / 2);
UsaColors usaColors;
RainbowColors rainbowColors(1, 1);
RainbowColors iteratingColors1(20, 20), iteratingColors2(20, 20);
SingleColor singleColor;

const int NUM_COLOR_FUNCTORS = 3;
ColorFunctor* colorFunctors[][2] = {
  //{&singleColor, &singleColor},
  //{&usaColors, &usaColors},
  {&rainbowColors, &rainbowColors},
  {&iteratingColors1, &iteratingColors2},
};


void setup() {
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS);
  FastLED.setBrightness(64);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  singleColor.color = CRGB::Green;
  Serial.begin(9600);
}


const int maxSleepTime_ms = 150;
const int minSleepTime_ms = 30;
const int sleepTimeIncrement_ms = 3;
int sleepTime_ms = minSleepTime_ms;
int sleepTimeStep_ms = -sleepTimeIncrement_ms;
uint8_t colorFunctorIndex = 0;

auto start_ms = millis();

void loop() {
  // Do animations here
  ColorFunctor* functor1 = colorFunctors[colorFunctorIndex][0];
  ColorFunctor* functor2 = colorFunctors[colorFunctorIndex][1];
  worm1.animate(*functor1);
  functor1->reset();
  worm2.animate(*functor2);
  functor2->reset();

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
    checkAndHandleButton();
  }
  FastLED.clear();

  if (start_ms + 30000 < millis()) {
    colorFunctorIndex = (colorFunctorIndex + 1) % NUM_COLOR_FUNCTORS;
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
    colorFunctorIndex = (colorFunctorIndex + 1) % NUM_COLOR_FUNCTORS;
  }
}
