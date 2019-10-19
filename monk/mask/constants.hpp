#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

const int MICROPHONE_ANALOG_PIN = A2;  // Digital pin 0
const int LED_COUNT = 45;

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#endif  // CONSTANTS_HPP
