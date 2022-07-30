#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

// Cast to int so that I don't get unsigned comparison warnings
#define COUNT_OF(x) static_cast<int>(sizeof((x)) / sizeof(0[(x)]))

// Regular sinf takes a value from 0 - 2*Pi. sin16 takes a value 0 - 65535
// representing a similar value. Multiply by this to convert.
const uint16_t PI_16_1_0 = 1.0 / (3.14159 * 2) * 65536;

const int SD_PIN = 5;

// Generated from offsets.py
#include "offsets.hpp"

#endif
