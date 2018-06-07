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


void setup() {
#ifdef __AVR_ATtiny85__  // Trinket, Gemma, etc.
  if(F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin();
  pixels.setBrightness(85);  // 1/3 brightness

  for (int i = 0; i < PIXEL_COUNT; ++i) {
    pixels.setPixelColor(i, 0);
  }
  pixels.show();

  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
}


uint32_t color = 0x0000FF;
void pickColor() {
  static uint32_t colorIndex = 0;
  const uint32_t colors[] = {0x0000FF, 0x00FF00, 0xFF0000, 0x00FFFF, 0xFF00FF, 0xFFFF00, 0xFFFFFF};

  setBrightness(20);
  delay(50);  // Debounce
  while (1) {
    // Wait for a button press
    while (digitalRead(BUTTON_PIN) != LOW);
    delay(50);
    while (digitalRead(BUTTON_PIN) != HIGH);

    // If it was a long press, we're done with colors
    delay(250);
    if (digitalRead(BUTTON_PIN) == HIGH) {
      return;
    }
    colorIndex = (colorIndex + 1) % (sizeof(colors) / sizeof(colors[0]));
    color = colors[colorIndex];
    setBrightness(20);
    delay(100);
    setBrightness(0);
  }
}


const int MODE_COUNT = 3;
enum mode_t {
  LIGHT,
  PARTY,
  KNIGHT_RIDER,
};
mode_t mode = mode_t::LIGHT;
void pickMode() {`
  setBrightness(20);
  delay(50);  // Debounce
  while (1) {
    // Wait for a button press
    while (digitalRead(BUTTON_PIN) != LOW);
    delay(50);
    while (digitalRead(BUTTON_PIN) != HIGH);

    // If it was a long press, we're done with modes
    delay(250);
    if (digitalRead(BUTTON_PIN) == HIGH) {
      return;
    }
    mode = (mode + 1) % MODE_COUNT;
    for (int i = 0; i < mode + 1; ++i) {
      setBrightness(20);
      delay(100);
      setBrightness(0);
      delay(100);
    }
  }
}


void setAllPixels(const uint8_t red, const uint8_t green, const uint8_t blue) {
  for (int i = 0; i < PIXEL_COUNT; ++i) {
    pixels.setPixelColor(i, red, green, blue);
  }
  pixels.show();
}


void setAllPixels(const uint32_t color, uint8_t brightness) {
  for (int i = 0; i < PIXEL_COUNT; ++i) {
    pixels.setPixelColor(
      i,
      (color >> 16) & brightness,
      (color >> 8) & brightness,
      (color & brightness)
    );
  }
  pixels.show();
}


void setBrightness(const uint8_t brightness) {
  setAllPixels(color, brightness);
}


void lightMode() {
  pixels.setBrightness(170);  // 2/3 brightness
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
  pixels.setBrightness(85);  // 1/3 brightness
}


void partyMode() {
  uint8_t colors[] = {255, 0, 0};
  for (int decColor = 0; decColor < 3; decColor += 1) {
    int incColor = decColor == 2 ? 0 : decColor + 1;

    // cross-fade the two colours
    for(int i = 0; i < 255; i += 1) {
      colors[decColor] -= 1;
      colors[incColor] += 1;

      setAllPixels(colors[0], colors[1], colors[2]);
      pixels.show();
      delay(5);
    }
  }
}


void knightRider() {
  for (int i = 0; i < PIXEL_COUNT; ++i) {
    for (int j = 0; j < PIXEL_COUNT; ++j) {
      pixels.setPixelColor(j, 0);
    }
    pixels.setPixelColor(i, color);
    pixels.show();
    delay(50);
  }
  for (int i = PIXEL_COUNT - 1; i >= 0; --i) {
    for (int j = 0; j < PIXEL_COUNT; ++j) {
      pixels.setPixelColor(j, 0);
    }
    pixels.setPixelColor(i, color);
    pixels.show();
    delay(50);
  }
}


void loop() {
begin:

  // Change color by holding the button
  delay(500);
  if (digitalRead(BUTTON_PIN) == HIGH) {
    pickColor();
    pickMode();
    while (digitalRead(BUTTON_PIN) != LOW);
  } else {
    switch (mode) {
      case mode_t::LIGHT:
        while (digitalRead(BUTTON_PIN) != HIGH);
        lightMode();
        break;
      case mode_t::PARTY:
        while (digitalRead(BUTTON_PIN) != HIGH) {
          partyMode();
        }
        break;
      case mode_t::KNIGHT_RIDER:
        while (digitalRead(BUTTON_PIN) != HIGH) {
          knightRider();
        }
        break;
      default:
        while (digitalRead(BUTTON_PIN) != HIGH);
        lightMode();
    }
  }
}
