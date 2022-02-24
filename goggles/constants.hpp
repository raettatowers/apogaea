#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstdint>

const int PIXEL_RING_COUNT = 20;
const int MICROPHONE_ANALOG_PIN = A1;

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((std::size_t)(!(sizeof(x) % sizeof(0[x])))))

#endif  // CONSTANTS_HPP
