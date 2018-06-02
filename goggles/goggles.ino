// Low power NeoPixel goggles example.  Makes a nice blinky display
// with just a few LEDs on at any time.

#include <Adafruit_NeoPixel.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
 #include <avr/power.h>
#endif
#include <arduinoFFT.h>

const int PIN = 0;
const int MODE_COUNT = 2;
const int SAMPLE_COUNT = 64; // Must be a power of 2
const int SAMPLING_FREQUENCY_HZ= 9600; // Must be less than 10000 due to ADC


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(32, PIN);

uint8_t mode   = 0; // Current animation effect
uint8_t offset = 0; // Position of spinny eyes
uint32_t color  = 0xFF0000; // Start red
uint32_t prevTime;

void setup() {
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
  if(F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin();
  pixels.setBrightness(85); // 1/3 brightness
  prevTime = millis();
}

void loop() {
  uint8_t i;
  uint32_t t;

  switch(mode) {

   case 0: // Random sparks - just one LED on at a time!
    i = random(32);
    pixels.setPixelColor(i, color);
    pixels.show();
    delay(10);
    pixels.setPixelColor(i, 0);
    break;
 
   case 1: // Spinny wheels (8 LEDs on at a time)
    for (i = 0; i < 16; i++) {
      uint32_t c = 0;
      if (((offset + i) & 7) < 2) {
        c = color; // 4 pixels on...
      }
      pixels.setPixelColor(   i, c); // First eye
      pixels.setPixelColor(31-i, c); // Second eye (flipped)
    }
    pixels.show();
    offset++;
    delay(50);
    break;
  }

  t = millis();
  if ((t - prevTime) > 8000) {
    mode++;
    if (mode > MODE_COUNT) {
      mode = 0;
      color >>= 8;
      if (!color) {
        color = 0xFF0000;
      }
    }
    for (i = 0; i < 32; i++) {
      pixels.setPixelColor(i, 0);
    }
    prevTime = t;
  }
}
