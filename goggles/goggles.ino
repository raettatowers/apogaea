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
const int INITIAL_BRIGHTNESS = 20;

Adafruit_DotStar internalPixel = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_RING_COUNT * 2, NEOPIXELS_PIN);

uint16_t hue = 0;  // Start red
bool reset;  // Indicates that an animation should clear its state


void setup() {
#ifdef __AVR_ATtiny85__  // Trinket, Gemma, etc.
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin();
  pixels.setBrightness(INITIAL_BRIGHTNESS);
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
  pixels->fill(0, 0, PIXEL_RING_COUNT * 2);
}


void configureBrightness(const bool buttonPressed) {
  constexpr uint8_t BRIGHTNESSES[] = {5, 10, 15, 20, 40, 60, 100, 150, 200};
  const uint8_t INITIAL_INDEX = 3;
  static_assert(INITIAL_BRIGHTNESS == BRIGHTNESSES[INITIAL_INDEX], "");
  static uint8_t brightnessIndex = INITIAL_INDEX;

  if (buttonPressed) {
    ++brightnessIndex;
    if (brightnessIndex == COUNT_OF(BRIGHTNESSES)) {
      brightnessIndex = 0;
    }
  }

  pixels.setBrightness(BRIGHTNESSES[brightnessIndex]);
  // Turn on an LED for each brightness
  pixels.fill(0x700000, 0, brightnessIndex + 1);
  pixels.fill(0, brightnessIndex + 1, PIXEL_RING_COUNT * 2 - brightnessIndex);
  pixels.show();
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
void rotateGearsToBeat(Adafruit_NeoPixel* pixels, uint16_t hue);

// Each animationFunction_t[] should end in nullptr
const animationFunction_t ONLY_ANIMATIONS[] = {spinnyWheels, binaryClock, fadingSparks, ripples, shimmer, swirls, nullptr};
const animationFunction_t ONLY_SPECTRUM_ANALYZER[] = {spectrumAnalyzer, nullptr};
const animationFunction_t ONLY_BEAT_DETECTIONS[] = {flashLensesToBeat, rotateGearsToBeat, nullptr};
const animationFunction_t* ANIMATIONS_LIST[] = {ONLY_ANIMATIONS, ONLY_SPECTRUM_ANALYZER, ONLY_BEAT_DETECTIONS};
// Use this for testing a single animation
//const animationFunction_t TEST_ANIMATION[] = {rotateGearsToBeat, nullptr};
//const animationFunction_t* ANIMATIONS_LIST[] = {TEST_ANIMATION};


typedef void (*configurationFunction_t)(bool buttonPressed);
const configurationFunction_t CONFIGURATION_FUNCTIONS[] = {configureBrightness, nullptr};


void loop() {
  static auto modeStartTime_ms = millis();
  static decltype(millis()) buttonToggleTime = 0;
  static bool buttonDown = false;
  static uint8_t mode = 0;  // Current animation effect
  static uint8_t animationsIndex = 0;
  static uint8_t configurationsIndex = COUNT_OF(CONFIGURATION_FUNCTIONS) - 1;

  bool buttonPress = false;
  bool longButtonPress = false;

  // Check for button presses
  const auto now = millis();
  // Debounce
  if (now - buttonToggleTime > 20) {
    if (!buttonDown && digitalRead(BUTTON_PIN) == HIGH) {
      buttonToggleTime = now;
      buttonDown = true;
    } else if (buttonDown && digitalRead(BUTTON_PIN) == LOW) {
      if (now - buttonToggleTime > 250) {
        longButtonPress = true;
      } else {
        buttonPress = true;
      }
      buttonToggleTime = now;
      buttonDown = false;
    }
  }

  if (buttonPress) {
    // If we're not configuring, go to the next animation set
    if (CONFIGURATION_FUNCTIONS[configurationsIndex] == nullptr) {
      ++animationsIndex;
      if (animationsIndex == COUNT_OF(ANIMATIONS_LIST)) {
        animationsIndex = 0;
      }
      mode = 0;
    } else {
      CONFIGURATION_FUNCTIONS[configurationsIndex](buttonPress);
      return;
    }
  } else if (longButtonPress) {
    ++configurationsIndex;
    if (configurationsIndex == COUNT_OF(CONFIGURATION_FUNCTIONS)) {
      configurationsIndex = 0;
    }
  }

  if (CONFIGURATION_FUNCTIONS[configurationsIndex] != nullptr) {
    CONFIGURATION_FUNCTIONS[configurationsIndex](buttonPress);
    // Return to stop doing animations
    return;
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
    reset = true;
    modeStartTime_ms = now_ms;
  } else {
    reset = false;
  }
}
