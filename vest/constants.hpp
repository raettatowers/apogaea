#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

// Cast to int so that I don't get unsigned comparison warnings
#define COUNT_OF(x) static_cast<int>(sizeof((x)) / sizeof(0[(x)]))

// Generated from offsets.py
#include "offsets.hpp"

#endif
