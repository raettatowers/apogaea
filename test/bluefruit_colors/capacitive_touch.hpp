// Modeled after Circuit Python's touchio module
class TouchIn {
  public:
    TouchIn(int pin);
    ~TouchIn() = default;
    TouchIn(const TouchIn&) = delete;
    TouchIn(const TouchIn&&) = delete;
    uint16_t getRawReading() const;
    bool touched() const;
  private:
    const int pin;
};

static const int SAMPLE_COUNT = 10;
static const int TIMEOUT_TICKS = 10000;

TouchIn::TouchIn(const int pin_) : pin(pin_) {
}

uint16_t TouchIn::getRawReading() const {
  uint16_t ticks = 0;
  for (uint16_t i = 0; i < SAMPLE_COUNT; ++i) {
    // Set pad to digital output high for 10us to charge it
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    delayMicroseconds(10);

    // Set pad back to an input and take some samples
    pinMode(pin, INPUT_PULLDOWN);
    while (digitalRead(pin) == 0) {
      if (ticks >= TIMEOUT_TICKS) {
        return TIMEOUT_TICKS;
      }
      ++ticks;
    }
  }
  return ticks;
}

bool TouchIn::touched() const {
  return getRawReading() == 0;
}
