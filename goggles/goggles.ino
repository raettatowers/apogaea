#include <FastLED.h>

#include "constants.hpp"

// On Uno, if you're using Serial, this needs to be > 1. On Trinket it should be 0.
const int LED_PIN = 0;
const int BUTTON_PIN = 4;
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

static uint8_t hue = 0;

bool reset;  // Indicates that an animation should clear its state
CRGB pixels[PIXEL_RING_COUNT * 2];
CRGB dotStar;

void setup() {
  FastLED.addLeds<NEOPIXEL, LED_PIN>(pixels, PIXEL_RING_COUNT * 2);
  FastLED.addLeds<APA102, 7, 8, BGR>(&dotStar, 1);
  FastLED.setBrightness(INITIAL_BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  analogReference(AR_DEFAULT);
  pinMode(MICROPHONE_ANALOG_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), setButtonDownState, CHANGE);
}


void setButtonDownState() {
  buttonDown = (digitalRead(BUTTON_PIN) == LOW);
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

  FastLED.setBrightness(BRIGHTNESSES[brightnessIndex]);
  // Turn on an LED for each brightness
  fill_solid(&pixels[0], brightnessIndex + 1, CRGB(0x700000));
  fill_solid(&pixels[brightnessIndex + 1], PIXEL_RING_COUNT * 2 - brightnessIndex, CRGB::Black);
  FastLED.show();
}


typedef void (*animationFunction_t)(uint8_t hue);

// Static animations
#define ANIM(name) void name(uint8_t hue)
ANIM(binaryClock);
ANIM(circularWipe);
ANIM(fadingSparks);
ANIM(lookAround);
ANIM(newtonsCradle);
ANIM(pacMan);
ANIM(rainbowSwirls);
ANIM(randomSparks);
ANIM(ripples);
ANIM(shimmer);
ANIM(spectrumAnalyzer);
ANIM(spinnyWheels);
ANIM(swirls);

// Beat detection animations
ANIM(flashLensesToBeat);
ANIM(rotateGearsToBeat);

// Each animationFunction_t[] should end in nullptr
constexpr animationFunction_t ANIMATIONS[] = {
  spinnyWheels,
  binaryClock,
  newtonsCradle,
  rainbowSwirls,
  fadingSparks,
  lookAround,
  ripples,
  circularWipe,
  shimmer,
  swirls,
  pacMan,
  nullptr
};
static_assert(ANIMATIONS[COUNT_OF(ANIMATIONS) - 1] == nullptr, "");
const constexpr animationFunction_t SPECTRUM_ANALYZER[] = {spectrumAnalyzer, nullptr};
static_assert(SPECTRUM_ANALYZER[COUNT_OF(SPECTRUM_ANALYZER) - 1] == nullptr, "");
const constexpr animationFunction_t FLASH_LENSES[] = {flashLensesToBeat, nullptr};
static_assert(FLASH_LENSES[COUNT_OF(FLASH_LENSES) - 1] == nullptr, "");
const constexpr animationFunction_t BEAT_DETECTIONS[] = {rotateGearsToBeat, flashLensesToBeat, nullptr};
static_assert(BEAT_DETECTIONS[COUNT_OF(BEAT_DETECTIONS) - 1] == nullptr, "");
const constexpr animationFunction_t* ANIMATIONS_LIST[] = {ANIMATIONS, SPECTRUM_ANALYZER, FLASH_LENSES, BEAT_DETECTIONS};
// Use this for testing a single animation
//constexpr animationFunction_t TEST_ANIMATION[] = {lookAround, nullptr};
//constexpr animationFunction_t* ANIMATIONS_LIST[] = {TEST_ANIMATION};


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
    FastLED.clear();
  }

  if (CONFIGURATION_FUNCTIONS[configurationsIndex] != nullptr) {
    CONFIGURATION_FUNCTIONS[configurationsIndex](buttonState == ButtonState_t::BUTTON_PRESS);
  } else {
    // Do a regular animation
    const auto startAnimation_ms = millis();
    const animationFunction_t* animations = ANIMATIONS_LIST[animationsIndex];
    // To keep the transitions smooth, we track hue using a 16-bit number, but
    // FastLED expects an 8-bit hue, so convert
    animations[mode](hue >> 8);

    // Update the color
    const auto now_ms = millis();
    // We want to complete a full hue color cycle about every X seconds
    const int hueCycle_ms = 20000;
    const int hueCycleLimit = 0xFFFF;
    hue += (now_ms - startAnimation_ms) * hueCycleLimit / hueCycle_ms;

    if ((now_ms - modeStartTime_ms) > MODE_TIME_MS) {
      ++mode;
      if (animations[mode] == nullptr) {
        mode = 0;
      }
      FastLED.clear();
      reset = true;
      modeStartTime_ms = now_ms;
    } else {
      reset = false;
    }
  }
}
