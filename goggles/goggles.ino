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
const int MODE_COUNT = 2;
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
uint32_t prevTime;

void setup() {
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
  if(F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin();
  pixels.setBrightness(85); // 1/3 brightness
  prevTime = millis();

  analogReference(EXTERNAL);
  pinMode(2, INPUT); // Microphone
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
  // Random takes too much ROM space, so use a lookup table
  // Also, using millis instead of micros for the index adds 20 bytes for some reason
  int8_t i = (((uint8_t*)&randomSparks)[micros() & 0xFF]) % (PIXEL_RING_COUNT * 2);
  pixels.setPixelColor(i, color);
  pixels.show();
  delay(10);
  pixels.setPixelColor(i, 0);
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

void showMicrophoneReading() {
  
}

void loop() {

  uint8_t i;
  uint32_t t;

  switch(mode) {
    case 0: // Random sparks - just one LED on at a time!
      randomSparks();
      break;

    case 1:
      visualizeAudio();
      break;
 
    case 2: // Spinny wheels (8 LEDs on at a time)
      spinnyWheels();
      break;
  }

  t = millis();
  if ((t - prevTime) > 8000) {
    mode++;
    if (mode >= MODE_COUNT) {
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
