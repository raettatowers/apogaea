#include <FastLED.h>

const int NUM_LEDS = 22;
CRGB countLeds[NUM_LEDS];
CRGB brightnessLeds[NUM_LEDS];
CRGB snakeLeds[NUM_LEDS];
const int MAX_LEDS_AT_ONCE = 8;
const int COUNT_LED_PIN = 2;
const int BRIGHTNESS_LED_PIN = 3;
const int SNAKE_LED_PIN = 4;

#define COUNT_OF(x) (sizeof(x) / sizeof(0[x]))

void setup() {
  Serial.begin(57600);
  Serial.println("resetting");
  LEDS.addLeds<WS2812, COUNT_LED_PIN, RGB>(&countLeds[0], COUNT_OF(countLeds));
  LEDS.addLeds<WS2812, BRIGHTNESS_LED_PIN, RGB>(&brightnessLeds[0], COUNT_OF(brightnessLeds));
  LEDS.addLeds<WS2812, SNAKE_LED_PIN, RGB>(&snakeLeds[0], COUNT_OF(snakeLeds));
  // Set to max brightness and rely on color to scale the brightness
  LEDS.setBrightness(255);
  LEDS.clear();
}

static uint8_t hue = 0;
auto color = CHSV(hue, 255, 32);
const auto BLACK = CRGB(0, 0, 0);

void loop() {
  const int MILLIS_PER_HUE = 50;
  static auto previousMillis = millis();
  if (millis() > previousMillis + MILLIS_PER_HUE) {
    previousMillis = millis();
    // Default to 64/256 == 1/4 brightness
    color = CHSV(hue, 255, 64);
    ++hue;
  }

  showCount();
  showBrightness();
  showSnake(4);
  FastLED.show();
}


void showCount() {
  const int MILLIS_PER_ITERATION = 500;
  static auto previousMillis = MILLIS_PER_ITERATION;
  static int index = 0;

  // Turn off all the LEDs
  fill_solid(countLeds, COUNT_OF(countLeds), BLACK);

  countLeds[index] = color;

  const auto now = millis();
  if (now > previousMillis + MILLIS_PER_ITERATION) {
    previousMillis = now;

    ++index;
    if (index >= COUNT_OF(countLeds)) {
      index = 0;
    }
  }
}


void showSnake(int snakeLength) {
  const int MILLIS_PER_ITERATION = 100;
  static auto previousMillis = MILLIS_PER_ITERATION;
  static int startIndex = 0;
  static int endIndex = 1;

  if (snakeLength <= 0) {
    return;
  }
  // Turn off all the LEDs
  fill_solid(snakeLeds, COUNT_OF(countLeds), BLACK);

  bool needUpdate = false;
  const auto now = millis();
  if (now > previousMillis + MILLIS_PER_ITERATION) {
    previousMillis = now;
    needUpdate = true;
    Serial.print("snake startIndex:");
    Serial.print(startIndex);
    Serial.print(" endIndex:");
    Serial.println(endIndex);
  }

  // Snake just entering
  if (endIndex < snakeLength) {
    for (int i = 0; i < endIndex; ++i) {
      snakeLeds[i] = color;
    }

    if (needUpdate) {
      ++endIndex;
    }
  } else {
    // Snake in the middle or exiting
    for (int i = startIndex; i < min(COUNT_OF(snakeLeds), startIndex + snakeLength); ++i) {
      snakeLeds[i] = color;
    }

    if (needUpdate) {
      ++startIndex;
      if (startIndex >= COUNT_OF(snakeLeds)) {
        startIndex = 0;
        endIndex = 1;
      }
    }
  }
}


void showBrightness() {
  const int MILLIS_PER_ITERATION = 4000;
  static auto previousMillis = MILLIS_PER_ITERATION;
  static int index = 0;
  const uint8_t brightnesses[] = {255, 128, 64, 32, 16};
  static_assert(COUNT_OF(brightnesses) <= COUNT_OF(brightnessLeds));

  // Turn off all the LEDs
  fill_solid(brightnessLeds, COUNT_OF(countLeds), BLACK);

  const auto color = CHSV(hue, 255, brightnesses[index]);
  for (int i = COUNT_OF(brightnesses); i > COUNT_OF(brightnesses) - index - 1 && i >= 0; --i) {
    brightnessLeds[i] = color;
  }

  const auto now = millis();
  if (now > previousMillis + MILLIS_PER_ITERATION) {
    previousMillis = now;
    ++index;
    if (index >= COUNT_OF(brightnesses)) {
      index = 0;
    }
  }
}
