#include <Adafruit_NeoPixel.h>
#include <Adafruit_DotStar.h>

#include "constants.hpp"

// On Uno, if you're using Serial, this needs to be > 1. On Trinket it should be 0.
const int NEOPIXELS_PIN = 0;
const int BUTTON_PIN = 3;
const int ONBOARD_LED_PIN = 13;
const int MODE_TIME_MS = 8000;
const int INITIAL_BRIGHTNESS = 20;

// State machine for button presses
enum class ButtonState_t {
  UP,
  DEBOUNCE_DOWN,
  DOWN,
  LONG_BUTTON_PRESS,
  BUTTON_PRESS,
  AFTER_BUTTON_PRESS,
};
static decltype(millis()) buttonDownTime = 0;
static ButtonState_t buttonState = ButtonState_t::UP;
static bool buttonDown = false;

static Adafruit_DotStar internalPixel = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);
static Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_RING_COUNT * 2, NEOPIXELS_PIN);
static uint16_t hue = 0;  // Start red
bool reset;  // Indicates that an animation should clear its state


void setup() {
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
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(NEOPIXELS_PIN, OUTPUT);
  pinMode(ONBOARD_LED_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), setButtonDownState, CHANGE);
}


void setButtonDownState() {
  buttonDown = (digitalRead(BUTTON_PIN) == HIGH);
}


void updateButtonState() {
  const auto now = millis();
  switch (buttonState) {
    case ButtonState_t::UP:
      if (buttonDown) {
        buttonState = ButtonState_t::DEBOUNCE_DOWN;
        buttonDownTime = millis();
      }
      break;
    case ButtonState_t::DEBOUNCE_DOWN:
      if (buttonDown && now - buttonDownTime > 20) {
        buttonState = ButtonState_t::DOWN;
      } else if (!buttonDown) {
        buttonState = ButtonState_t::UP;
      }
      break;
    case ButtonState_t::DOWN:
      if (now - buttonDownTime > 500) {
        buttonState = ButtonState_t::LONG_BUTTON_PRESS;
      } else if (!buttonDown) {
        buttonState = ButtonState_t::BUTTON_PRESS;
      }
      break;
    case ButtonState_t::LONG_BUTTON_PRESS:
      buttonState = ButtonState_t::AFTER_BUTTON_PRESS;
      break;
    case ButtonState_t::BUTTON_PRESS:
      buttonState = ButtonState_t::AFTER_BUTTON_PRESS;
      break;
    case ButtonState_t::AFTER_BUTTON_PRESS:
      if (!buttonDown) {
        buttonState = ButtonState_t::UP;
      }
      break;
  }
}


static void clearLeds(Adafruit_NeoPixel* pixels) {
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

// Static animations
void binaryClock(Adafruit_NeoPixel* pixels, uint16_t hue);
void fadingSparks(Adafruit_NeoPixel* pixels, uint16_t hue);
void newtonsCradle(Adafruit_NeoPixel* pixels, uint16_t hue);
void rainbowSwirls(Adafruit_NeoPixel* pixels, uint16_t hue);
void randomSparks(Adafruit_NeoPixel* pixels, uint16_t hue);
void ripples(Adafruit_NeoPixel* pixels, uint16_t hue);
void shimmer(Adafruit_NeoPixel* pixels, uint16_t hue);
void spectrumAnalyzer(Adafruit_NeoPixel* pixels, uint16_t hue);
void spinnyWheels(Adafruit_NeoPixel* pixels, uint16_t hue);
void swirls(Adafruit_NeoPixel* pixels, uint16_t hue);

// Beat detection animations
void flashLensesToBeat(Adafruit_NeoPixel* pixels, uint16_t hue);
void rotateGearsToBeat(Adafruit_NeoPixel* pixels, uint16_t hue);

// Each animationFunction_t[] should end in nullptr
const animationFunction_t ANIMATIONS[] = {
  spinnyWheels,
  binaryClock,
  newtonsCradle,
  rainbowSwirls,
  fadingSparks,
  ripples,
  shimmer,
  swirls,
  nullptr
};
const animationFunction_t SPECTRUM_ANALYZER[] = {spectrumAnalyzer, nullptr};
const animationFunction_t FLASH_LENSES[] = {flashLensesToBeat, nullptr};
const animationFunction_t BEAT_DETECTIONS[] = {rotateGearsToBeat, flashLensesToBeat, nullptr};
const animationFunction_t* ANIMATIONS_LIST[] = {ANIMATIONS, SPECTRUM_ANALYZER, FLASH_LENSES, BEAT_DETECTIONS};
// Use this for testing a single animation
//const animationFunction_t TEST_ANIMATION[] = {rainbowSwirls, nullptr};
//const animationFunction_t* ANIMATIONS_LIST[] = {TEST_ANIMATION};


typedef void (*configurationFunction_t)(bool buttonPressed);
const configurationFunction_t CONFIGURATION_FUNCTIONS[] = {configureBrightness, nullptr};


void loop() {
  static auto modeStartTime_ms = millis();
  static uint8_t mode = 0;  // Current animation effect
  static uint8_t animationsIndex = 0;
  static uint8_t configurationsIndex = COUNT_OF(CONFIGURATION_FUNCTIONS) - 1;

  setButtonDownState();
  updateButtonState();
  if (buttonState == ButtonState_t::BUTTON_PRESS) {
    // If we're not configuring, go to the next animation set
    if (CONFIGURATION_FUNCTIONS[configurationsIndex] == nullptr) {
      ++animationsIndex;
      if (animationsIndex == COUNT_OF(ANIMATIONS_LIST)) {
        animationsIndex = 0;
      }
      mode = 0;
    }
  } else if (buttonState == ButtonState_t::LONG_BUTTON_PRESS) {
    ++configurationsIndex;
    if (configurationsIndex == COUNT_OF(CONFIGURATION_FUNCTIONS)) {
      configurationsIndex = 0;
    }
    clearLeds(&pixels);
  }

  if (CONFIGURATION_FUNCTIONS[configurationsIndex] != nullptr) {
    CONFIGURATION_FUNCTIONS[configurationsIndex](buttonState == ButtonState_t::BUTTON_PRESS);
  } else {
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
}
