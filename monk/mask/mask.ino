#include <Adafruit_DotStar.h>
#include <FastLED.h>

#include "constants.hpp"

static const int LED_PIN = 2;
static const int BUTTON_PIN = 3;
static const int ONBOARD_LED_PIN = 13;

static CRGB leds[LED_COUNT];
static Adafruit_DotStar internalPixel = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);


void setup() {
  FastLED.addLeds<WS2812B, LED_PIN>(leds, LED_COUNT);
  FastLED.setBrightness(1.0 * 255);

  analogReference(AR_DEFAULT);
  pinMode(MICROPHONE_ANALOG_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(BUTTON_PIN, HIGH);
  pinMode(ONBOARD_LED_PIN, OUTPUT);

  internalPixel.begin();
  internalPixel.setBrightness(5);
  // Blink it for a few seconds, so that if I screw something up,
  // I can still upload code for a little bit
  uint32_t colors[] = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00};
  for (int i = 0; i < 3; ++i) {
    for (auto color : colors) {
      internalPixel.setPixelColor(0, color);
      internalPixel.show();
      delay(250);
    }
  }
  // Just shut off the internal pixel
  internalPixel.setPixelColor(0, 0);
  internalPixel.show();
  delay(250);

  Serial.begin(9600);
}


void loop() {
  while (true) {
    uint8_t hue = 0;
    setBrightnessOnSound();
    ++hue;
  }
}


void showLedOnButtonPress() {
  if (digitalRead(BUTTON_PIN) == HIGH) {
    digitalWrite(ONBOARD_LED_PIN, LOW);
  } else {
    digitalWrite(ONBOARD_LED_PIN, HIGH);
  }
  delay(50);
}


void setBrightnessOnSound() {
  const int SAMPLE_RATE = 16000;
  const int MICROS_PER_SAMPLE = 1000000 / SAMPLE_RATE;

  static uint32_t baseLineAverage = 0;

  // First get a baseline
  if (baseLineAverage == 0) {
    const int BASE_SAMPLE_COUNT = 128;
    for (int i = 0; i < BASE_SAMPLE_COUNT; ++i) {
      const auto start = micros();
      const auto microphoneValue = analogRead(MICROPHONE_ANALOG_PIN);
      baseLineAverage += microphoneValue;
      Serial.print(microphoneValue, DEC);
      Serial.print(" ");
      Serial.println();
      while (micros() < start + MICROS_PER_SAMPLE);
    }
    baseLineAverage /= BASE_SAMPLE_COUNT;
  }

  const int SAMPLE_COUNT = 32;
  uint32_t sum = 0;
  for (int i = 0; i < SAMPLE_COUNT; ++i) {
    const auto start = micros();
    sum += analogRead(MICROPHONE_ANALOG_PIN);
    while (micros() < start + MICROS_PER_SAMPLE);
  }
  sum /= SAMPLE_COUNT;
  const int16_t brightness = baseLineAverage - sum * 1.2;
  const uint8_t constrainedBrightness = brightness >= 0 ? (brightness <= 255 ? brightness : 255) : 0;
  internalPixel.setBrightness(255);
  internalPixel.setPixelColor(0, internalPixel.ColorHSV(0, 0xFF, constrainedBrightness));
  internalPixel.show();
  delay(50);
}


void oneAtATime(const uint8_t hue) {
  static uint8_t led = 9;

  if (Serial.available() > 0) {
    char arr[5];
    Serial.readBytesUntil('\n', arr, COUNT_OF(arr) - 1);
    while (Serial.available() > 0) {
      Serial.read();
    }
    arr[COUNT_OF(arr) - 1] = '\0';
    led = min(atoi(arr), LED_COUNT);
  }

  FastLED.clear();
  leds[led] = CRGB::White;
  FastLED.show();
  delay(10);
}


void oneAtATimeCycle(const uint8_t hue) {
  static uint8_t led = 0;
  static bool rainbow = true;
  static uint32_t colorCodes[] = {0x0000FF, 0x00FF00, 0x00FFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF};
  static uint8_t colorIndex = 0;
  static typeof(millis()) colorTime_ms;

  const typeof(millis()) colorTimeout_ms = 3 * 1000;
  const typeof(millis()) rainbowTimeout_ms = 8 * 1000;

  if (Serial.available() > 0) {
    char arr[5];
    Serial.readBytesUntil('\n', arr, COUNT_OF(arr) - 1);
    while (Serial.available() > 0) {
      Serial.read();
    }
    arr[COUNT_OF(arr) - 1] = '\0';
    led = min(atoi(arr), LED_COUNT);
  }

  FastLED.clear();
  if (rainbow) {
    leds[led] = CHSV(hue, 0xFF, 0xFF);

    if (millis() - colorTime_ms > rainbowTimeout_ms) {
      colorTime_ms = millis();
      rainbow = false;
    }
  } else {
    leds[led] = CRGB(colorCodes[colorIndex]);

    if (millis() - colorTime_ms > colorTimeout_ms) {
      colorTime_ms = millis();
      ++colorIndex;
      if (colorIndex >= COUNT_OF(colorCodes)) {
        colorIndex = 0;
        rainbow = true;
      }
    }
  }
  FastLED.show();
  delay(10);
}


void rainbowRipple(const uint8_t hue) {
  const uint8_t rippleLength = 4;
  const uint8_t brightness[rippleLength] = {30, 60, 120, 240};
  static uint8_t rippleStart = 0;

  FastLED.clear();
  for (uint8_t i = 0; i < rippleLength; ++i) {
    const int index = (rippleStart + i) % LED_COUNT;
    const int rippleHue = hue + i;
    leds[index].setHSV(rippleHue, 0xFF, brightness[i]);
  }
  FastLED.show();
  delay(40);
  rippleStart = (rippleStart + 1) % LED_COUNT;
}


void breathe(const uint8_t hue) {
  static uint8_t brightness = 1;
  static decltype(millis()) zeroTimeout_ms = 0;
  static uint8_t step = 1;

  const decltype(millis()) zeroPauseTime_ms = 1000;

  FastLED.clear();
  if (zeroTimeout_ms > 0) {
    if (millis() > zeroTimeout_ms) {
      zeroTimeout_ms = 0;
    }
  } else {
    brightness += step;
    if (brightness == 255) {
      step = -1;
    } else if (brightness == 0) {
      step = 1;
      zeroTimeout_ms = millis() + zeroPauseTime_ms;
    }
    for (auto& value : leds) {
      value.setHSV(hue, 0xFF, brightness);
    }
  }
  FastLED.show();
  delay(10);
}
