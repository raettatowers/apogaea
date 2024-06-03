#define COUNT_OF(x) (sizeof(x) / sizeof(0[x]))

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

const int LEDS_PER_STRIP = 150;
const int STRIP_COUNT = 6;

const int c4Index = 11;

constexpr int LED_PINS[] = {14, 18, 19, 23, 33, 34};
static_assert(COUNT_OF(LED_PINS) == STRIP_COUNT);
