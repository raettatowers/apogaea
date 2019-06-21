// Low power NeoPixel goggles example.  Makes a nice blinky display
// with just a few LEDs on at any time.

#include <Adafruit_NeoPixel.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
 #include <avr/power.h>
#endif
#include <arduinoFFT.h>

const int PIN = 0;
const int ONBOARD_LED = 1;
const int MICROPHONE_ANALOG_PIN = 1;
const int MODE_COUNT = 3;
const int SAMPLE_COUNT = 32; // Must be a power of 2
const int SAMPLING_FREQUENCY_HZ = 9600; // Must be less than 10000 due to ADC
const int UPDATES_PER_SECOND = 50;
const int PIXEL_RING_COUNT = 16;
const unsigned int sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY_HZ));

double vReal[SAMPLE_COUNT];
double vImaginary[SAMPLE_COUNT];

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(32, PIN);
arduinoFFT FFT = arduinoFFT();

uint8_t mode = 0; // Current animation effect
uint32_t color = 0xFF0000; // Start red


void setup() {
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
  if(F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin();
  pixels.setBrightness(85); // 1/3 brightness

  analogReference(EXTERNAL);
  pinMode(ONBOARD_LED, OUTPUT);
}


void visualizeAudio() {
  uint32_t microseconds;
  for (int i = 0; i < SAMPLE_COUNT; ++i) {
    microseconds = micros();
    vReal[i] = analogRead(MICROPHONE_ANALOG_PIN);
    vImaginary[i] = 0;
    while (micros() < (microseconds + sampling_period_us));
  }
  FFT.Windowing(vReal, SAMPLE_COUNT, FFT_WIN_TYP_RECTANGLE, FFT_FORWARD);
  FFT.Compute(vReal, vImaginary, SAMPLE_COUNT, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImaginary, SAMPLE_COUNT);

  for (int i = 0; i < PIXEL_RING_COUNT * 2; ++i) {
    const uint8_t intensity = max(vReal[i], 255);
    pixels.setPixelColor(i, intensity, intensity, intensity);
  }
  delay(10);
}


void randomSparks() {
  static const uint8_t table[] = {15, 6, 0, 7, 12, 3, 5, 4, 8, 2, 1, 14, 13, 11, 10, 9};
  const uint8_t led1 = table[millis() & 0x0F];
  pixels.setPixelColor(led1, color);
  const uint8_t led2 = table[millis() & 0x0F] + PIXEL_RING_COUNT;
  pixels.setPixelColor(led2, color);
  pixels.show();
  delay(10);
  pixels.setPixelColor(led1, 0);
  pixels.setPixelColor(led2, 0);
}


void binaryClock() {
  // Do a binary shift instead of integer division because of speed and code size.
  // It's a little less precise, but who cares.
  // Show 1/4 seconds instead of full seconds because it's more interesting. It
  // updates a lot faster and the other lens lights up faster.
  uint32_t now = millis() >> 8;
  showNumber(now, color);
  delay(100);
  clearLeds();
}


void spinnyWheels() {
  static uint8_t offset = 0; // Position of spinny eyes
  for (int i = 0; i < PIXEL_RING_COUNT; i++) {
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
}


void showNumber(uint32_t number, const uint32_t color) {
  uint8_t counter = 0;
  while (number > 0) {
    if (number & 1) {
      pixels.setPixelColor(counter, color);
    } else {
      pixels.setPixelColor(counter, 0);
    }
    number >>= 1;
    counter += 2;
  }
  pixels.show();
}


void clearLeds() {
  for (int i = 0; i < 32; i++) {
    pixels.setPixelColor(i, 0);
  }
}


void loop() {
  static uint32_t prevTime = millis();
  uint32_t t;

  switch(mode) {
    case 0: // Random sparks - just one LED on at a time!
      randomSparks();
      break;

    case 1:
      binaryClock();
      break;
 
    case 2: // Spinny wheels (8 LEDs on at a time)
      spinnyWheels();
      break;

    /*
    case 3:
      visualizeAudio();
      break;
      */
  }

  static uint8_t index = 0;
  // x = reduce(lambda a, b: a + b, ([i] * 3 for i in range(7))); import random; random.shuffle(x); print(x)
  const static uint8_t color_indexes[] = {2, 1, 0, 3, 0, 6, 5, 1, 4, 1, 0, 5, 3, 2, 6, 3, 4, 5, 2, 6, 4};
  const static uint32_t colors[] = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF, 0xFFFFFF};

  t = millis();
  if ((t - prevTime) > 8000) {
    color = colors[color_indexes[index]];
    index = (index + 1) % (sizeof(color_indexes) / sizeof(color_indexes[0]));
    mode++;
    if (mode >= MODE_COUNT) {
      mode = 0;
    }
    clearLeds();
    prevTime = t;
  }
}
