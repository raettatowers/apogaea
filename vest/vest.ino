#include <FastLED.h>

const int NUM_LEDS = 20;
CRGB leds[NUM_LEDS];
const int MAX_LEDS_AT_ONCE = 8;
const int LED_PIN = 2;

void setup() {
  Serial.begin(57600);
  Serial.println("resetting");
  LEDS.addLeds<WS2812, LED_PIN, RGB>(&leds[0], NUM_LEDS);
  LEDS.setBrightness(64);
  LEDS.clear();
}

void loop() {
  LEDS.setBrightness(128);
  showSnake(4, 10000);
  LEDS.setBrightness(32);
  showColors(1, 5000);
  LEDS.setBrightness(64);
  showColors(2, 5000);
  LEDS.setBrightness(128);
  showColors(4, 5000);
  LEDS.setBrightness(255);
  showColors(8, 5000);
}

static uint8_t hue = 0;
void showColors(int skip, int milliseconds) {
  if (skip <= 0) {
    return;
  }
  LEDS.clear();
  const auto start = millis();
  while (millis() - start < milliseconds) {
    for (int i = 0; i < MAX_LEDS_AT_ONCE; i += skip) {
      leds[i] = CHSV(hue, 255, 255);
      hue += skip;
      FastLED.show();
      delay(50 * skip);
    }
  }
  LEDS.clear();
}

void showSnake(int snakeLength, int milliseconds) {
  if (snakeLength <= 0) {
    return;
  }
  LEDS.clear();
  const auto start = millis();
  while (millis() - start < milliseconds) {
    for (int i = 0; i < NUM_LEDS - snakeLength; ++i) {
      for (int j = i; j < i + snakeLength; ++j) {
        leds[j] = CHSV(hue, 255, 255);
        FastLED.show();
      }
      delay(60);
      FastLED.clear();
      ++hue;
    }
  }
}
