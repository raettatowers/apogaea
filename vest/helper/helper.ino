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
uint8_t RemoteXY_CONF[] =   // 188 bytes
{ 255, 10, 0, 0, 0, 181, 0, 17, 0, 0, 0, 24, 1, 106, 200, 1, 1, 15, 0, 3,
  24, 4, 10, 44, 5, 2, 26, 129, 38, 6, 29, 7, 17, 87, 104, 105, 116, 101, 32, 49,
  55, 0, 129, 38, 15, 25, 7, 17, 66, 108, 117, 101, 32, 50, 49, 0, 129, 38, 23, 19,
  7, 17, 82, 101, 100, 32, 52, 0, 129, 38, 32, 25, 7, 17, 71, 114, 101, 101, 110, 32,
  48, 0, 129, 38, 41, 28, 7, 17, 66, 108, 97, 99, 107, 32, 49, 53, 0, 6, 37, 78,
  31, 31, 2, 26, 1, 14, 53, 24, 24, 0, 2, 31, 0, 7, 50, 61, 40, 10, 52, 2,
  26, 2, 4, 13, 125, 80, 10, 128, 2, 26, 129, 33, 114, 40, 8, 17, 66, 114, 105, 103,
  104, 116, 110, 101, 115, 115, 0, 1, 12, 166, 24, 24, 0, 2, 31, 0, 1, 70, 166, 24,
  24, 0, 2, 31, 0, 129, 12, 150, 24, 12, 17, 82, 111, 119, 0, 129, 59, 150, 43, 12,
  17, 67, 111, 108, 117, 109, 110, 0
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
  uint8_t row_button; // =1 if button pressed, else =0
  uint8_t column_button; // =1 if button pressed, else =0

  // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;
#pragma pack(pop)

/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

#include <FastLED.h>
#include "offsets.hpp"

// How many leds in your strip?
const int LED_COUNT = 120;
CRGB leds[5][LED_COUNT];

void setup() {
  RemoteXY_Init();
  RemoteXY.pin_select = 0;
  RemoteXY.brightness_slider = 25;

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
  static int previousRowButton = 0;
  static int previousColumnButton = 0;
  static int row = 0;
  static int column = 0;

  FastLED.clear();

  const auto color = CRGB(RemoteXY.rgb_r, RemoteXY.rgb_g, RemoteXY.rgb_b);

  // This will only give us a range from 0-200 instead of 0-255, but that's fine for testing
  FastLED.setBrightness(RemoteXY.brightness_slider * 2);

  // Manual stuff
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
  leds[RemoteXY.pin_select][RemoteXY.led_number_field] = color;

  // Row and column stuff
  if (RemoteXY.row_button) {
    row = (row + 1) % LED_ROW_COUNT;
    RemoteXY.row_button = 0;
  }
  if (RemoteXY.column_button) {
    column = (column + 1) % LED_COLUMN_COUNT;
    RemoteXY.column_button = 0;
  }

  for (int r = 0; r < LED_ROW_COUNT; ++r) {
    const uint8_t strip = xyToStrip[column][r];
    const uint8_t offset = xyToOffset[column][r];
    leds[strip][offset] = color;
  }
  for (int c = 0; c < LED_COLUMN_COUNT; ++c) {
    const uint8_t strip = xyToStrip[c][row];
    const uint8_t offset = xyToOffset[c][row];
    leds[strip][offset] = color;
  }

  FastLED.show();
}
