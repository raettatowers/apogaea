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


void setAllPixels(const uint8_t red, const uint8_t green, const uint8_t blue) {
  for (int i = 0; i < 8; ++i) {
    pixels.setPixelColor(i, red, green, blue);
  }
  pixels.show();
}


void setAllPixels(const uint32_t color, uint8_t brightness) {
  for (int i = 0; i < 8; ++i) {
    pixels.setPixelColor(
      i,
      (color >> 16) & brightness,
      (color >> 8) & brightness,
      (color & brightness)
    );
  }
  pixels.show();
}


uint32_t colorIndex = 0;
const int COLOR_COUNT = 7;
const uint32_t colors[COLOR_COUNT] = {0x0000FF, 0x00FF00, 0xFF0000, 0x00FFFF, 0xFF00FF, 0xFFFF00, 0xFFFFFF};


void setBrightness(const uint8_t brightness) {
  setAllPixels(colors[colorIndex], brightness);
}


void loop() {
begin:
  while (digitalRead(BUTTON_PIN) != HIGH);

  // Change color by holding the button
  delay(500);
  if (digitalRead(BUTTON_PIN) == HIGH) {
    // Mode change!
    ++colorIndex;
    if (colorIndex > COLOR_COUNT) {
      colorIndex = 0;
    }
    setBrightness(40);
    delay(50);
    setBrightness(0);
    delay(1000);
    goto begin;
  }

  // Flicker on
  for (int i = 0; i < 2; ++i) {
    setBrightness(100);
    delay(20);
    setBrightness(0);
    delay(50);
  }
  delay(250);
  setBrightness(100);
  delay(20);
  setBrightness(0);
  delay(350);

  // Fade in
  for (int brightness = 0; brightness < MAX_BRIGHTNESS; ++brightness) {
    setBrightness(brightness);
    delay(10);
  }

  while (digitalRead(BUTTON_PIN) != HIGH);

  for (int brightness = MAX_BRIGHTNESS; brightness >= 1; --brightness) {
    setBrightness(brightness);
    delay(10);
  }
  setBrightness(0);
}
