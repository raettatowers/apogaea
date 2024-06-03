// To compile:
// arduino-cli compile --fqbn esp32:esp32:esp32da piddle.ino
// To upload:
// arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32da
// I don't know how to set frequency and stuff easily, might need to use the Arduino IDE

#include <arduinoFFT.h>
#include <FastLED.h>

// RemoteXY select connection mode and include library
#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>

#include <RemoteXY.h>

// RemoteXY connection settings
#define REMOTEXY_BLUETOOTH_NAME "Piddle"


// RemoteXY configurate
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = // 136 bytes
  { 255,5,0,21,0,129,0,16,24,1,4,128,6,11,52,5,2,26,129,0,
  15,3,31,6,17,84,114,101,98,108,101,32,110,111,116,101,0,129,0,16,
  19,31,6,17,66,114,105,103,104,116,110,101,115,115,0,129,0,17,36,29,
  6,17,83,101,110,115,105,116,105,118,105,116,121,0,4,128,6,28,52,5,
  2,26,4,128,6,43,52,5,2,26,3,3,46,54,8,22,2,26,129,0,
  8,62,35,6,17,84,114,101,98,108,101,32,114,97,110,103,101,0,1,0,
  26,82,12,12,2,31,0,67,4,4,73,53,6,2,26,21 };

// this structure defines all the variables and events of your control interface
struct {

    // input variables
  int8_t trebleSlider; // =0..100 slider position
  int8_t brightnessSlider; // =0..100 slider position
  int8_t sensitivitySlider; // =0..100 slider position
  uint8_t trebleRangeSelect; // =0 if select position A, =1 if position B, =2 if position C, ...
  uint8_t buttonTest; // =1 if button pressed, else =0

    // output variables
  char text[21];  // string UTF8 end zero

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;
#pragma pack(pop)


#include "constants.hpp"

void spectrumAnalyzer();
void setupSpectrumAnalyzer();
void testLeds();
void blink(const int delay_ms = 500);

CRGB leds[STRIP_COUNT][LEDS_PER_STRIP];

void buttonInterrupt() {
  // This space intentionally left blank. Kept for further expansion.
}

void setup() {
  Serial.begin(115200);
  #warning "Serial is enabled"

  // This had to be done first, but I think I fixed the bug that was causing problems? I don't want
  // to test if it's fixed, so I'm leaving it first now
  setupSpectrumAnalyzer();

  //analogReference(AR_DEFAULT); // Not on ESP32?
  pinMode(LED_BUILTIN, OUTPUT);

  RemoteXY_Init();
  RemoteXY.trebleSlider = 50;
  RemoteXY.brightnessSlider = 64;
  RemoteXY.sensitivitySlider = 50;
  RemoteXY.trebleRangeSelect = 1;

  FastLED.addLeds<WS2812B, LED_PINS[0], GRB>(leds[0], LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, LED_PINS[1], GRB>(leds[1], LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, LED_PINS[2], GRB>(leds[2], LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, LED_PINS[3], GRB>(leds[3], LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, LED_PINS[4], GRB>(leds[4], LEDS_PER_STRIP);
  FastLED.setBrightness(RemoteXY.brightnessSlider);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 5000);

  // The boot button is connected to GPIO0
  pinMode(0, INPUT);
  attachInterrupt(0, buttonInterrupt, FALLING);

  for (int i = 0; i < 3; ++i) {
    blink(100);
  }
}

extern float minimumDivisor;
extern int startTrebleNote;
extern int additionalTrebleRange;
extern float vReal[];
void loop() {
  RemoteXY_Handler();
  FastLED.setBrightness(RemoteXY.brightnessSlider);
  constexpr int low = 500;
  constexpr int high = 20500;
  constexpr int mid = (low + high) / 2;
  minimumDivisor = mid + (-RemoteXY.sensitivitySlider + 50) * (high - low) / 100; 
  startTrebleNote = c4Index + (RemoteXY.trebleSlider - 50) / 10;
  additionalTrebleRange = RemoteXY.trebleRangeSelect;

  if (RemoteXY.buttonTest) {
    testLeds();
  } else {
    spectrumAnalyzer();
    FastLED.show();
  }

  spectrumAnalyzer();
}

void blink(const int delay_ms) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(delay_ms);
  digitalWrite(LED_BUILTIN, LOW);
  delay(delay_ms);
}

void testLeds() {
  FastLED.clear();
  for (int i = 0; i < STRIP_COUNT; ++i) {
    leds[i][5] = CRGB::Red;
    leds[i][25] = CRGB::Red;
    leds[i][50] = CRGB::Red;
    leds[i][LEDS_PER_STRIP - 10] = CRGB::Red;
  }
  FastLED.show();

  RemoteXY.text[sizeof(RemoteXY.text) - 1] = '\0';
  delay(20);
}
