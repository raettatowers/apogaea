#include <Adafruit_DotStar.h>
#include <FastLED.h>

#include "constants.hpp"

// Constants
static const int LED_PIN = 2;
static const int BUTTON_PIN = 1;
static const int ONBOARD_LED_PIN = 13;
static const int INITIAL_BRIGHTNESS = 100;
static const int MILLIS_PER_HUE = 100;

// Function declarations
void setButtonDownState();

// State machine for button presses
enum class ButtonState_t {
  UP,
  DEBOUNCE_DOWN,
  DOWN,
  LONG_BUTTON_PRESS,
  BUTTON_PRESS,
  AFTER_BUTTON_PRESS,
};

// Static global variables
static Adafruit_DotStar internalPixel = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);
static decltype(millis()) buttonDownTime = 0;
static ButtonState_t buttonState = ButtonState_t::UP;
static bool buttonDown = false;

// Non-static global variables
bool reset = true;  // Indicates that an animation should clear its state
CRGB leds[LED_COUNT];


void setup() {
  Serial.begin(9600);

  FastLED.addLeds<WS2812B, LED_PIN>(leds, LED_COUNT);
  FastLED.setBrightness(0.25 * 255);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1000);

  analogReference(AR_DEFAULT);
  pinMode(MICROPHONE_ANALOG_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(ONBOARD_LED_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), setButtonDownState, CHANGE);

  internalPixel.begin();
  internalPixel.setBrightness(5);
  // Pause if the button is pressed so that if I mess something up,
  // I can still upload code and recover
  if (digitalRead(BUTTON_PIN) == LOW) {
    uint32_t colors[] = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00};
    for (int i = 0; i < 10; ++i) {
      for (auto color : colors) {
        internalPixel.setPixelColor(0, color);
        internalPixel.show();
        delay(250);
      }
    }
  }
  // Just shut off the internal pixel
  internalPixel.setPixelColor(0, 0);
  internalPixel.show();
}


typedef void (*animationFunction_t)(uint8_t hue);
// Static animations
#define ANIM(name) void name(uint8_t hue)
ANIM(binaryClock);
ANIM(breathe);
ANIM(circularWipe);
ANIM(fadingSparks);
ANIM(pacMan);
ANIM(rainbowSwirl);
ANIM(shimmer);
ANIM(spectrumAnalyzer);


// Each animationFunction_t[] should end in nullptr
constexpr animationFunction_t ANIMATIONS[] = {
  breathe,
  //binaryClock,
  circularWipe,
  fadingSparks,
  //pacMan,
  rainbowSwirl,
  shimmer,
  spectrumAnalyzer
};

// Use this for testing a single animation
//constexpr animationFunction_t ANIMATIONS[] = {pacMan};


typedef void (*configurationFunction_t)(bool buttonPressed);
const constexpr configurationFunction_t CONFIGURATION_FUNCTIONS[] = {configureBrightness, nullptr};
static_assert(CONFIGURATION_FUNCTIONS[COUNT_OF(CONFIGURATION_FUNCTIONS) - 1] == nullptr, "");


void loop() {
  static uint8_t animationsIndex = 0;
  static uint8_t configurationsIndex = COUNT_OF(CONFIGURATION_FUNCTIONS) - 1;
  static uint8_t hue = 0;
  static auto hueChangeTime_ms = millis();

  setButtonDownState();
  updateButtonState();
  if (buttonState == ButtonState_t::BUTTON_PRESS) {
    // If we're not configuring, go to the next animation
    if (CONFIGURATION_FUNCTIONS[configurationsIndex] == nullptr) {
      ++animationsIndex;
      if (animationsIndex == COUNT_OF(ANIMATIONS)) {
        animationsIndex = 0;
      }
      fill_solid(&leds[0], LED_COUNT, CRGB::Black);
      reset = true;  // Animation functions need to clear reset (if they care)
    }
  } else if (buttonState == ButtonState_t::LONG_BUTTON_PRESS) {
    ++configurationsIndex;
    if (configurationsIndex == COUNT_OF(CONFIGURATION_FUNCTIONS)) {
      configurationsIndex = 0;
    }
    fill_solid(&leds[0], LED_COUNT, CRGB::Black);
  }

  if (CONFIGURATION_FUNCTIONS[configurationsIndex] != nullptr) {
    CONFIGURATION_FUNCTIONS[configurationsIndex](buttonState == ButtonState_t::BUTTON_PRESS);
  } else {
    Serial.printf("Doing animation %d\n", animationsIndex);
    // Do a regular animation
    ANIMATIONS[animationsIndex](hue);

    // Update the color
    const auto now_ms = millis();
    if (now_ms >= hueChangeTime_ms + MILLIS_PER_HUE) {
      hue += (now_ms - hueChangeTime_ms) / MILLIS_PER_HUE;
      hueChangeTime_ms = now_ms;
    }
  }
}


void setButtonDownState() {
  buttonDown = (digitalRead(BUTTON_PIN) == LOW);
  digitalWrite(LED_BUILTIN, buttonDown ? HIGH : LOW);
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
  const uint8_t INITIAL_INDEX = 6;
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
  fill_solid(&leds[0], brightnessIndex + 1, CRGB(0x700000));
  fill_solid(&leds[brightnessIndex + 1], LED_COUNT - brightnessIndex, CRGB::Black);
  FastLED.show();
}
