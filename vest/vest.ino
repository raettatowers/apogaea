#include <FastLED.h>

const int NUM_LEDS = 40;
CRGB leds[NUM_LEDS];
const int MAX_LEDS_AT_ONCE = 8;
const int LED_PIN = 7;
const int BUTTON_PIN = 11;


void setup() {
  Serial.begin(9600);
  Serial.println("resetting");
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  LEDS.addLeds<WS2812, LED_PIN, RGB>(&leds[0], COUNT_OF(leds));
  // Set to max brightness and rely on color to scale the brightness
  LEDS.setBrightness(255);
  LEDS.clear();
}

static uint8_t hue = 0;
auto color = CHSV(hue, 255, 32);
const (*animations[])(void) = {showCount, showSnake, showBrightness};
static uint8_t animationIndex = 0;


void loop() {
  const int millisPerHue = 50;
  static auto previousMillis = millis();
  static CRGB color = CHSV(hue, 255, 64);

  if (millis() > previousMillis + millisPerHue) {
    previousMillis = millis();
    // Default to 64/256 == 1/4 brightness
    color = CHSV(hue, 255, 64);
    ++hue;
  }

  animations[animationIndex]();
  if (buttonPressed()) {
    animationIndex = (animationIndex + 1) % COUNT_OF(animations);
    Serial.print("New index: ");
    Serial.println(animationIndex);
  }

  FastLED.show();
}


void showCount() {
  const int MILLIS_PER_ITERATION = 500;
  static auto previousMillis = MILLIS_PER_ITERATION;
  static int index = 0;

  // Turn off all the LEDs
  fill_solid(leds, COUNT_OF(leds), CRGB::Black);

  leds[index] = color;

  const auto now = millis();
  if (now > previousMillis + MILLIS_PER_ITERATION) {
    previousMillis = now;

    ++index;
    if (index >= COUNT_OF(leds)) {
      index = 0;
    }
  }
}


void showSnake() {
  const int MILLIS_PER_ITERATION = 100;
  static auto previousMillis = MILLIS_PER_ITERATION;
  static int startIndex = 0;
  static int endIndex = 1;
  const int snakeLength = 5;

  if (snakeLength <= 0) {
    return;
  }
  // Turn off all the LEDs
  fill_solid(leds, COUNT_OF(leds), CRGB::Black);

  bool needUpdate = false;
  const auto now = millis();
  if (now > previousMillis + MILLIS_PER_ITERATION) {
    previousMillis = now;
    needUpdate = true;
  }

  // Snake just entering
  if (endIndex < snakeLength) {
    for (int i = 0; i < endIndex; ++i) {
      leds[i] = color;
    }

    if (needUpdate) {
      ++endIndex;
    }
  } else {
    // Snake in the middle or exiting
    for (int i = startIndex; i < min(COUNT_OF(leds), startIndex + snakeLength); ++i) {
      leds[i] = color;
    }

    if (needUpdate) {
      ++startIndex;
      if (startIndex >= COUNT_OF(leds)) {
        startIndex = 0;
        endIndex = 1;
      }
    }
  }
}


void showBrightness() {
  const int MILLIS_PER_ITERATION = 4000;
  static auto previousMillis = MILLIS_PER_ITERATION;
  static int index = 0;
  const uint8_t brightnesses[] = {255, 128, 64, 32, 16};
  static_assert(COUNT_OF(brightnesses) <= COUNT_OF(leds));

  // Turn off all the LEDs
  fill_solid(leds, COUNT_OF(leds), CRGB::Black);

  const auto color = CHSV(hue, 255, brightnesses[index]);
  for (int i = COUNT_OF(brightnesses); i > COUNT_OF(brightnesses) - index - 1 && i >= 0; --i) {
    leds[i] = color;
  }

  const auto now = millis();
  if (now > previousMillis + MILLIS_PER_ITERATION) {
    previousMillis = now;
    ++index;
    if (index >= COUNT_OF(brightnesses)) {
      index = 0;
    }
  }
}


bool buttonPressed() {
  static int debounceTime_ms = 0;
  static bool pressed = false;

  if (millis() - debounceTime_ms < 25) {
    // Just a bounce, ignore it
    return false;
  }

  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == HIGH) {
    // Not pressed
    pressed = false;
    digitalWrite(LED_BUILTIN, LOW);
    return false;
  }

  debounceTime_ms = millis();
  // Avoid continual presses
  if (pressed) {
    return false;
  }
  pressed = true;
  digitalWrite(LED_BUILTIN, HIGH);
  return true;
}
