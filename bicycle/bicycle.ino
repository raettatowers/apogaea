#include <FastLED.h>
#include <Adafruit_CircuitPlayground.h>

#include "animations.hpp"
#include "constants.hpp"
// Define the array of leds
CRGB leds[NUM_LEDS];

RainbowColors iteratingColors(20, 20);
SpectrumAnalyzer analyzer(leds, NUM_LEDS);

void setup() {
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS);
  FastLED.setBrightness(64);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CPLAY_REDLED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);
  Serial.begin(9600);
  CircuitPlayground.begin();
}


const int maxSleepTime_ms = 150;
const int minSleepTime_ms = 30;
const int sleepTimeIncrement_ms = 3;
int sleepTime_ms = minSleepTime_ms;
int sleepTimeStep_ms = -sleepTimeIncrement_ms;
uint8_t colorFunctorIndex = 0;

void loop() {
  // Do animations here

  FastLED.show();
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  analyzer.animate(iteratingColors);
}


void checkAndHandleButton() {
  static int debounceTime_ms = 0;
  static bool pressed = false;

  int buttonState = digitalRead(BUTTON_PIN);
}
