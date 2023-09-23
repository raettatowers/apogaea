#define COUNT_OF(x) (sizeof(x) / sizeof(0[x]))

const int LEDS_PER_STRIP = 100;
const int STRIP_COUNT = 5;

const int BUTTON_PIN = 34;
const int MICROPHONE_ANALOG_PIN = 26;

const int c4Note = 15;

constexpr int LED_PINS[] = {23, 22, 19, 4, 15};
static_assert(COUNT_OF(LED_PINS) == STRIP_COUNT);
