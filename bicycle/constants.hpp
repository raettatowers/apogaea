#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <Adafruit_CircuitPlayground.h>

// How many leds in your strip?
const int LED_COUNT = 57;

// A1 = D6, A2 = D9, A3 = D10, A4 = D3
const int CLOCK_PIN = 10;
const int DATA_PIN = 3;

const int BUTTON_PIN = 4;  // Button A on Circuit Playground Bluefruit

#define COUNT_OF(x) (sizeof(x) / sizeof(x[0]))

#endif
