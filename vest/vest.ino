#include <FastLED.h>

#include "animations.hpp"
#include "constants.hpp"

static CRGB leds[LED_COUNT];
static const int MAX_LEDS_AT_ONCE = 8;

static uint8_t hue = 0;

static Animation* animations[] = {
  &Count(leds, LED_COUNT),
  &Snake(leds, LED_COUNT),
  &ShowBrightness(leds, LED_COUNT),
};
static int animationIndex = 0;


void setup() {
  // Do a delay in case I do something stupid, like immediately put
  // it into sleep mode, which will make it impossible to program
  delay(2000);

  Serial.begin(57600);
  Serial.println("resetting");
  const int ledPin = 2;
  LEDS.addLeds<WS2812, ledPin, RGB>(&leds[0], LED_COUNT);
  // Set to max brightness and rely on color to scale the brightness
  LEDS.setBrightness(255);
  LEDS.clear();
}


void loop() {
  const int millisPerHue = 50;
  static auto previousMillis = millis();
  static CRGB color = CHSV(hue, 255, 64);

  if (millis() > previousMillis + millisPerHue) {
    previousMillis = millis();
    // Default to 64/256 == 1/4 brightness
    color = CHSV(hue, 255, 64);
    ++hue;
  }

  animation[animationIndex].animate(color);
  FastLED.show();
}
