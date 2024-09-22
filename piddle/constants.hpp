#define COUNT_OF(x) (sizeof(x) / sizeof(0[x]))

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

const int LEDS_PER_STRIP = 300;
const int STRIP_COUNT = 5;

const int c4Index = 11;

constexpr int LED_PINS[] = {32, 33, 25, 26, 27};
static_assert(COUNT_OF(LED_PINS) == STRIP_COUNT);
