// To compile:
// arduino-cli compile --fqbn esp32:esp32:esp32doit-devkit-v1 piddle.ino
// To upload:
// arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32doit-devkit-v1 piddle.ino

#include <arduinoFFT.h>
#include <FastLED.h>

/*
// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>
#include <RemoteXY.h>

#define REMOTEXY_BLUETOOTH_NAME "Piddle"

// RemoteXY configurate  
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // 69 bytes
  { 255,7,0,0,0,62,0,16,24,1,4,128,6,20,52,5,2,26,4,128,
  6,28,52,5,2,26,1,0,6,4,12,12,2,31,0,1,0,25,4,12,
  12,2,31,0,1,0,45,4,12,12,2,31,0,4,128,6,36,52,5,2,
  26,3,8,6,46,7,49,2,26 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  int8_t slider_1; // =0..100 slider position 
  int8_t slider_2; // =0..100 slider position 
  uint8_t button_1; // =1 if button pressed, else =0 
  uint8_t button_2; // =1 if button pressed, else =0 
  uint8_t button_3; // =1 if button pressed, else =0 
  int8_t slider_3; // =0..100 slider position 
  uint8_t select_1; // =0 if select position A, =1 if position B, =2 if position C, ... 

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0 

} RemoteXY;
#pragma pack(pop)
*/

#include "constants.hpp"

void spectrumAnalyzer();
void setupSpectrumAnalyzer(int arduinoCore);

CRGB leds[STRIP_COUNT][LEDS_PER_STRIP];

void blink(const int delay_ms = 500) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(delay_ms);
  digitalWrite(LED_BUILTIN, LOW);
  delay(delay_ms);
}

void setup() {
  // // Disable this before compiling!
  // Serial.begin(115200);
  // #warning "Serial is enabled"

  //RemoteXY_Init(); 

  //analogReference(AR_DEFAULT); // Not on ESP32?
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MICROPHONE_ANALOG_PIN, INPUT);

  FastLED.addLeds<WS2812B, LED_PINS[0], GRB>(leds[0], LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, LED_PINS[1], GRB>(leds[1], LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, LED_PINS[2], GRB>(leds[2], LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, LED_PINS[3], GRB>(leds[3], LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, LED_PINS[4], GRB>(leds[4], LEDS_PER_STRIP);
  FastLED.setBrightness(16);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1000);

  setupSpectrumAnalyzer(xPortGetCoreID());

  for (int i = 0; i < 3; ++i) {
    blink(100);
  }
}

void loop() {
  //RemoteXY_Handler();

  //FastLED.clear();
  spectrumAnalyzer();
  FastLED.show();
}
