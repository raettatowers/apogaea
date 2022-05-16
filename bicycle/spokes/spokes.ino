#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>
#include <FastLED.h>
#include <RemoteXY.h>

#include "animations.hpp"

// RemoteXY connection settings
#define REMOTEXY_BLUETOOTH_NAME "spokes"

#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // 451 bytes
{ 255, 10, 0, 0, 0, 188, 1, 16, 24, 1, 129, 0, 10, 3, 14, 3, 17, 79, 117, 116,
  101, 114, 32, 104, 117, 101, 0, 129, 0, 10, 7, 17, 3, 17, 79, 117, 116, 101, 114, 32,
  114, 105, 112, 112, 108, 101, 0, 129, 0, 10, 11, 14, 3, 17, 80, 101, 110, 100, 117, 108,
  117, 109, 0, 129, 0, 10, 15, 7, 3, 17, 79, 114, 98, 105, 116, 0, 129, 0, 10, 19,
  16, 3, 17, 84, 114, 105, 97, 100, 32, 111, 114, 98, 105, 116, 115, 0, 129, 0, 10, 23,
  19, 3, 17, 66, 108, 117, 114, 114, 101, 100, 32, 115, 112, 105, 114, 97, 108, 0, 129, 0,
  10, 27, 27, 3, 17, 66, 108, 117, 114, 114, 101, 100, 32, 115, 112, 105, 114, 97, 108, 32,
  104, 117, 101, 115, 0, 129, 0, 10, 31, 30, 3, 17, 70, 97, 100, 105, 110, 103, 32, 114,
  105, 110, 103, 115, 0, 129, 0, 10, 35, 19, 3, 17, 67, 111, 109, 101, 116, 115, 32, 115,
  104, 111, 114, 116, 0, 129, 0, 10, 39, 18, 3, 17, 67, 111, 109, 101, 116, 115, 32, 108,
  111, 110, 103, 0, 3, 10, 3, 2, 4, 40, 2, 26, 3, 5, 3, 42, 4, 20, 2, 26,
  129, 0, 10, 43, 27, 3, 17, 79, 117, 116, 119, 97, 114, 100, 32, 114, 105, 112, 112, 108,
  101, 32, 104, 117, 101, 0, 129, 0, 10, 47, 18, 3, 17, 83, 105, 110, 103, 108, 101, 32,
  115, 112, 105, 114, 97, 108, 0, 129, 0, 10, 51, 21, 3, 17, 79, 117, 116, 119, 97, 114,
  100, 32, 114, 105, 112, 112, 108, 101, 0, 129, 0, 10, 55, 18, 3, 17, 79, 117, 116, 119,
  97, 114, 100, 32, 104, 117, 101, 0, 129, 0, 10, 59, 16, 3, 17, 73, 110, 119, 97, 114,
  100, 32, 104, 117, 101, 0, 2, 1, 43, 2, 18, 7, 2, 26, 31, 31, 67, 121, 99, 108,
  101, 0, 83, 116, 97, 116, 105, 99, 0, 129, 0, 3, 65, 21, 4, 17, 66, 114, 105, 103,
  104, 116, 110, 101, 115, 115, 0, 129, 0, 4, 70, 20, 4, 17, 67, 121, 99, 108, 101, 32,
  116, 105, 109, 101, 0, 4, 128, 26, 64, 32, 5, 2, 26, 4, 128, 26, 69, 32, 5, 2,
  26, 129, 0, 12, 75, 12, 4, 17, 83, 112, 101, 101, 100, 0, 4, 128, 26, 74, 32, 5,
  2, 26, 2, 1, 43, 12, 18, 7, 2, 26, 31, 31, 65, 110, 105, 109, 0, 83, 111, 108,
  105, 100, 0, 6, 0, 41, 21, 20, 20, 2, 26
};
// this structure defines all the variables and events of your control interface
struct {

  // input variables
  uint8_t animationSelect1; // =0 if select position A, =1 if position B, =2 if position C, ...
  uint8_t animationSelect2; // =0 if select position A, =1 if position B, =2 if position C, ...
  uint8_t cycleSwitch; // =1 if switch ON and =0 if OFF
  int8_t brightnessSlider; // =0..100 slider position
  int8_t cycleTimeSlider; // =0..100 slider position
  int8_t speedSlider; // =0..100 slider position
  uint8_t animationSwitch; // =1 if switch ON and =0 if OFF
  uint8_t rgb_r; // =0..255 Red color value
  uint8_t rgb_g; // =0..255 Green color value
  uint8_t rgb_b; // =0..255 Blue color value

  // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;
#pragma pack(pop)

const int LED_PIN = 4;

CRGB leds[LED_COUNT];

int (* const animations[])() = {
  outerHue,
  outerRipple,
  pendulum,
  orbit,
  triadOrbits,
  blurredSpiral,
  blurredSpiralHues,
  fadingRainbowRings,
  cometsShort,
  comets,
  outwardRipple,
  outwardRippleHue,
  singleSpiral,
  spiral,
  lightAll,
  fastOutwardHue,
  fastInwardHue
};

void setup() {
  RemoteXY_Init();
  pinMode(LED_PIN, OUTPUT);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_COUNT).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(64);
  FastLED.clear();
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 300);

  // Some animations look bad when first called but then settle down, so just
  // call each animation a few times to let them settle
  for (unsigned int i = 0; i < COUNT_OF(animations); ++i) {
    for (int j = 0; j < 20; ++j) {
      animations[i]();
    }
  }

  RemoteXY.brightnessSlider = 25;
  RemoteXY.cycleTimeSlider = 50;
  RemoteXY.speedSlider = 25;
  RemoteXY.animationSwitch = 1;
  RemoteXY.cycleSwitch = 1;
  RemoteXY_Handler();
}

void loop() {
  const int minimumBrightness = 20;
  const decltype(millis()) minAnimationTime_ms = 5000;
  const decltype(millis()) maxAnimationTime_ms = 60000;

  static decltype(millis()) animationStart_ms = 0;
  static int animationIndex = 0;
  static uint8_t previousAnimationSelect1 = 0, previousAnimationSelect2 = 0;

  FastLED.clear();
  if (RemoteXY.animationSwitch == 1) {
    const float multiplier = 20.0f / (static_cast<float>(RemoteXY.speedSlider) + 5.0f);
    const decltype(millis()) delayTime_ms = animations[animationIndex]() * multiplier;
    const auto start = millis();
    while (millis() < start + delayTime_ms) {
      RemoteXY_Handler();
    }
  } else {
    fill_solid(leds, LED_COUNT, CRGB(RemoteXY.rgb_r, RemoteXY.rgb_g, RemoteXY.rgb_b));
    RemoteXY_Handler();
  }
  FastLED.show();

  // Handle the app changes
  if (previousAnimationSelect1 != RemoteXY.animationSelect1) {
    animationIndex =  RemoteXY.animationSelect1;
  } else if (previousAnimationSelect2 != RemoteXY.animationSelect2) {
    animationIndex =  RemoteXY.animationSelect2 + 10;
  }
  previousAnimationSelect1 = RemoteXY.animationSelect1;
  previousAnimationSelect2 = RemoteXY.animationSelect2;

  const decltype(millis()) animationDuration_ms = RemoteXY.cycleTimeSlider * (maxAnimationTime_ms - minAnimationTime_ms) / 100 + minAnimationTime_ms;
  if (RemoteXY.cycleSwitch == 1) {
    if (millis() > animationStart_ms + animationDuration_ms) {
      animationIndex = (animationIndex + 1) % COUNT_OF(animations);
      animationStart_ms = millis();
    }
  } else {
    // If we're not cycling, just keep resetting the start time
    animationStart_ms = millis();
  }

  FastLED.setBrightness(RemoteXY.brightnessSlider * (255 - minimumBrightness) / 100 + minimumBrightness);
}
