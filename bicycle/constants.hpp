#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <Adafruit_CircuitPlayground.h>

// How many leds in your strip?
const int NUM_LEDS = 57;

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
const int DATA_PIN = 7;  // D11 == A1
const int CLOCK_PIN = 8;  // D12 == A2

// For Circuit Playground Bluefruit
const int BUTTON_PIN = CPLAY_LEFTBUTTON;

// For else
//const int BUTTON_PIN = 11;

#define COUNT_OF(x) (sizeof(x) / sizeof(x[0]))

#endif
