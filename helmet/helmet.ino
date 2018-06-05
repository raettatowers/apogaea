// Low power NeoPixel goggles example.  Makes a nice blinky display
// with just a few LEDs on at any time.

#include <Adafruit_NeoPixel.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
 #include <avr/power.h>
#endif

const int LED_PIN = 3;
const int BUTTON_PIN = 1;
const int ONBOARD_LED = 1;
const int PIXEL_COUNT = 8;
const int MAX_BRIGHTNESS = 240;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(8, LED_PIN);

uint8_t mode = 0; // Current animation effect
uint32_t color = 0xFF0000; // Start red


void setup() {
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
  if(F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin();
  pixels.setBrightness(85); // 1/3 brightness

  for (int i = 0; i < PIXEL_COUNT; ++i) {
    pixels.setPixelColor(i, 0);
  }
  pixels.show();

  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
}


int waitForButtonPress() {
  const int DEBOUNCE_DELAY_MS = 50;
  static int previousBounceMs = -DEBOUNCE_DELAY_MS;

  while (millis() - previousBounceMs > DEBOUNCE_DELAY_MS);

  int state, previousButtonState;
  previousButtonState = state = digitalRead(BUTTON_PIN);
  while (state == previousButtonState) {
    state = digitalRead(BUTTON_PIN);
  }
  return state;
}


void loop() {
  while (waitForButtonPress() != HIGH);
  
  for (int brightness = 0; brightness < MAX_BRIGHTNESS; ++brightness) {
    for (int i = 0; i < 8; ++i) {
      pixels.setPixelColor(i, 0, 0, brightness);
    }
    pixels.show();
    delay(10);
  }

  while (waitForButtonPress() != HIGH);

  for (int brightness = MAX_BRIGHTNESS; brightness >= 10; --brightness) {
    for (int i = 0; i < 8; ++i) {
      pixels.setPixelColor(i, 0, 0, brightness);
    }
    pixels.show();
    delay(10);
  }
  for (int i = 0; i < 8; ++i) {
    pixels.setPixelColor(i, 0, 0, 0);
  }
  pixels.show();
}
