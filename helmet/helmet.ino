// Low power NeoPixel goggles example.  Makes a nice blinky display
// with just a few LEDs on at any time.

#include <Adafruit_NeoPixel.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
 #include <avr/power.h>
#endif

const int PIN = 3;
const int ONBOARD_LED = 1;
const int PIXEL_COUNT = 8;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(8, PIN);

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

  analogReference(EXTERNAL);
  pinMode(ONBOARD_LED, OUTPUT);
}


void loop() {
  digitalWrite(ONBOARD_LED, HIGH);
  delay(500);
  digitalWrite(ONBOARD_LED, LOW);
  for (int brightness = 10; brightness < 200; ++brightness) {
    for (int i = 0; i < 8; ++i) {
      pixels.setPixelColor(i, brightness, brightness, brightness);
    }
    pixels.show();
    delay(30);
  }
  for (int brightness = 200; brightness >= 10; --brightness) {
    for (int i = 0; i < 8; ++i) {
      pixels.setPixelColor(i, brightness, brightness, brightness);
    }
    pixels.show();
    delay(30);
  }
}
