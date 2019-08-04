#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstdint>

const int PIXEL_RING_COUNT = 16;

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((std::size_t)(!(sizeof(x) % sizeof(0[x])))))


#endif  // CONSTANTS_HPP
