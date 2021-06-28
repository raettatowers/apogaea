#include <FastLED.h>
#include "animations.hpp"

// How many leds in your strip?
#define NUM_LEDS 57

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 13
#define CLOCK_PIN 12

// Define the array of leds
CRGB leds[NUM_LEDS];

InchWormAnimation worm1(leds, NUM_LEDS, 9, 0);
InchWormAnimation worm2(leds, NUM_LEDS, 9, NUM_LEDS / 2);
//SingleColor singleColor;
UsaColors usaColors;

void setup() {
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS);
  FastLED.setBrightness(64);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  Serial.begin(9600);
}


const int maxSleepTime_ms = 150;
const int minSleepTime_ms = 30;
const int sleepTimeIncrement_ms = 3;
int sleepTime_ms = minSleepTime_ms;
int sleepTimeStep_ms = -sleepTimeIncrement_ms;

void loop() {
  // Do animations here
  worm1.animate(usaColors);
  usaColors.reset();
  worm2.animate(usaColors);
  usaColors.reset();
  
  FastLED.show();
  sleepTime_ms += sleepTimeStep_ms;
  if (sleepTime_ms >= maxSleepTime_ms) {
    sleepTimeStep_ms = -sleepTimeIncrement_ms;
  } else if (sleepTime_ms <= minSleepTime_ms) {
    sleepTimeStep_ms = sleepTimeIncrement_ms;
  }
  delay(sleepTime_ms);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}
