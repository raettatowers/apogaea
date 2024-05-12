// Helper program for configuring for my vest so that I can configure the LED positions.
// RmeoteXY program that lets me choose the LED strip and number to light up.
/*
   -- vest test --

   This source code of graphical user interface
   has been generated automatically by RemoteXY editor.
   To compile this code using RemoteXY library 3.1.10 or later version
   download by link http://remotexy.com/en/library/
   To connect using RemoteXY mobile app by link http://remotexy.com/en/download/
     - for ANDROID 4.13.11 or later version;
     - for iOS 1.10.3 or later version;

   This source code is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
*/

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// you can enable debug logging to Serial at 115200
//#define REMOTEXY__DEBUGLOG

// RemoteXY select connection mode and include library
#define REMOTEXY_MODE__ESP32CORE_BLE

#include <BLEDevice.h>

// RemoteXY connection settings
#define REMOTEXY_BLUETOOTH_NAME "vest test"


#include <RemoteXY.h>

// RemoteXY GUI configuration
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // 147 bytes
{ 255, 8, 0, 0, 0, 140, 0, 17, 0, 0, 0, 24, 1, 106, 200, 1, 1, 11, 0, 3,
  10, 9, 12, 56, 5, 2, 26, 129, 27, 10, 41, 10, 17, 87, 104, 105, 116, 101, 32, 49,
  55, 0, 129, 27, 21, 35, 10, 17, 66, 108, 117, 101, 32, 50, 49, 0, 129, 27, 33, 27,
  10, 17, 82, 101, 100, 32, 52, 0, 129, 27, 44, 36, 10, 17, 71, 114, 101, 101, 110, 32,
  48, 0, 129, 27, 55, 40, 10, 17, 66, 108, 97, 99, 107, 32, 49, 53, 0, 6, 36, 114,
  40, 40, 2, 26, 1, 14, 77, 24, 24, 0, 2, 31, 0, 7, 50, 84, 40, 10, 52, 2,
  26, 2, 4, 14, 180, 80, 10, 128, 2, 26, 129, 23, 163, 60, 12, 17, 66, 114, 105, 103,
  104, 116, 110, 101, 115, 115, 0
};

// this structure defines all the variables and events of your control interface
struct {

  // input variables
  uint8_t pin_select; // from 0 to 5
  uint8_t rgb_r; // =0..255 Red color value
  uint8_t rgb_g; // =0..255 Green color value
  uint8_t rgb_b; // =0..255 Blue color value
  uint8_t next_button; // =1 if button pressed, else =0
  int16_t led_number_field; // -32768 .. +32767
  int8_t brightness_slider; // from 0 to 100

  // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;
#pragma pack(pop)

/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

#include <FastLED.h>

// How many leds in your strip?
const int LED_COUNT = 120;
CRGB leds[5][LED_COUNT];

void setup() {
  RemoteXY_Init();
  RemoteXY.pin_select = 0;
  RemoteXY.brightness_slider = 50;

  Serial.begin(115200);
  Serial.println("resetting");
  const int pins[] = {17, 21, 4, 0, 15};
  for (const auto pin : pins) {
    pinMode(pin, OUTPUT);
  }
  FastLED.addLeds<WS2812, 17, GRB>(leds[0], LED_COUNT);
  FastLED.addLeds<WS2812, 21, GRB>(leds[1], LED_COUNT);
  FastLED.addLeds<WS2812, 4, GRB>(leds[2], LED_COUNT);
  FastLED.addLeds<WS2812, 0, GRB>(leds[3], LED_COUNT);
  FastLED.addLeds<WS2812, 15, GRB>(leds[4], LED_COUNT);
  FastLED.setBrightness(255);
  FastLED.clear();
}

void loop() {
  RemoteXY_Handler();

  static int previousButton = 0;
  static int previousPinSelect = 0;

  FastLED.clear();

  // This will only give us a range from 0-200 instead of 0-255, but that's fine for testing
  FastLED.setBrightness(RemoteXY.brightness_slider * 2);

  if (RemoteXY.next_button != previousButton) {
    Serial.printf("button:%d previous:%d\n", RemoteXY.next_button, previousButton);
  }

  if (previousPinSelect != RemoteXY.pin_select) {
    RemoteXY.led_number_field = 0;
  } else if (RemoteXY.next_button) {
    RemoteXY.led_number_field = (RemoteXY.led_number_field + 1) % LED_COUNT;
    RemoteXY.next_button = 0;
  }
  previousPinSelect = RemoteXY.pin_select;
  previousButton = RemoteXY.next_button;
  leds[RemoteXY.pin_select][RemoteXY.led_number_field] = CRGB(RemoteXY.rgb_r, RemoteXY.rgb_g, RemoteXY.rgb_b);
  FastLED.show();
}
