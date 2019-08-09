#include <Adafruit_NeoPixel.h>
#include <Adafruit_DotStar.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
#include <avr/power.h>
#endif
#include <fix_fft.h>

#include "constants.hpp"

// On Uno, if you're using Serial, this needs to be > 1. On Trinket it should be 0.
const int NEOPIXELS_PIN = 0;
const int BUTTON_PIN = 3;
const int ONBOARD_LED_PIN = 13;
const int MODE_TIME_MS = 8000;

Adafruit_DotStar internalPixel = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_RING_COUNT * 2, NEOPIXELS_PIN);

uint16_t hue = 0;  // Start red


void setup() {
#ifdef __AVR_ATtiny85__  // Trinket, Gemma, etc.
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin();
  pixels.setBrightness(20);
  clearLeds(&pixels);
  pixels.show();

  internalPixel.begin();
  // Just shut it off
  internalPixel.setPixelColor(0, 0);
  internalPixel.show();

  analogReference(AR_DEFAULT);
  pinMode(MICROPHONE_ANALOG_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(NEOPIXELS_PIN, OUTPUT);
  pinMode(ONBOARD_LED_PIN, OUTPUT);
}


void clearLeds(Adafruit_NeoPixel* pixels) {
  for (int i = 0; i < PIXEL_RING_COUNT * 2; ++i) {
    pixels->setPixelColor(i, 0);
  }
}


void configureBrightness(const bool buttonPressed) {
  static uint8_t targetBrightness = 1;

  if (buttonPressed) {
    pixels.setBrightness((256 / (PIXEL_RING_COUNT * 2)) * targetBrightness - 1);
  }

  pixels.setBrightness(10);
  for (int i = 0; i < PIXEL_RING_COUNT * 2; ++i) {
    if (i <= targetBrightness) {
      pixels.setPixelColor(i, 0x700000);
    }
  }
  pixels.show();
  delay(250);

  ++targetBrightness;
  if (targetBrightness == PIXEL_RING_COUNT * 2) {
    targetBrightness = 0;
  }
}

typedef void (*animationFunction_t)(Adafruit_NeoPixel* pixels, uint16_t hue);

void binaryClock(Adafruit_NeoPixel* pixels, uint16_t hue);
void fadingSparks(Adafruit_NeoPixel* pixels, uint16_t hue);
void ripples(Adafruit_NeoPixel* pixels, uint16_t hue);
void shimmer(Adafruit_NeoPixel* pixels, uint16_t hue);
void spectrumAnalyzer(Adafruit_NeoPixel* pixels, uint16_t hue);
void spinnyWheels(Adafruit_NeoPixel* pixels, uint16_t hue);
void swirls(Adafruit_NeoPixel* pixels, uint16_t hue);
void flashLensesToBeat(Adafruit_NeoPixel* pixels, uint16_t hue);

// Each animationFunction_t[] should end in nullptr
const animationFunction_t ONLY_ANIMATIONS[] = {spinnyWheels, binaryClock, fadingSparks, ripples, shimmer, swirls, nullptr};
const animationFunction_t ONLY_SPECTRUM_ANALYZER[] = {spectrumAnalyzer, nullptr};
const animationFunction_t ONLY_BEAT_DETECTIONS[] = {flashLensesToBeat, nullptr};
const animationFunction_t* ANIMATIONS_LIST[] = {ONLY_ANIMATIONS, ONLY_SPECTRUM_ANALYZER, ONLY_BEAT_DETECTIONS};
// Use this for testing a single animation
//const animationFunction_t TEST_ANIMATION[] = {flashLensesToBeat, nullptr};
//const animationFunction_t* ANIMATIONS_LIST[] = {TEST_ANIMATION};


void loop() {
  static uint32_t modeStartTime_ms = millis();
  static uint32_t buttonPressTime = 0;
  static uint8_t mode = 0;  // Current animation effect
  static uint8_t animationsIndex = 0;

  bool buttonPressed = false;

  if (digitalRead(BUTTON_PIN) == HIGH) {
    // Debounce
    if (millis() - buttonPressTime > 100) {
      buttonPressTime = millis();
      buttonPressed = true;
    }
  }

  if (buttonPressed) {
    // TODO: More complex configuration
    ++animationsIndex;
    if (animationsIndex == COUNT_OF(ANIMATIONS_LIST)) {
      animationsIndex = 0;
    }
    mode = 0;
  }

  // Do a regular animation
  const uint32_t startAnimation_ms = millis();
  const animationFunction_t* animations = ANIMATIONS_LIST[animationsIndex];
  animations[mode](&pixels, hue);

  // Update the color
  const uint32_t now_ms = millis();
  // We want to complete a full hue color cycle about every X seconds
  const int hueCycle_ms = 20000;
  const int hueCycleLimit = 65535;
  hue += (now_ms - startAnimation_ms) * hueCycleLimit / hueCycle_ms;

  if ((now_ms - modeStartTime_ms) > MODE_TIME_MS) {
    ++mode;
    if (animations[mode] == nullptr) {
      mode = 0;
    }
    clearLeds(&pixels);
    modeStartTime_ms = now_ms;
  }
}
