// Low power NeoPixel goggles example.  Makes a nice blinky display
// with just a few LEDs on at any time.

#include <Adafruit_NeoPixel.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
 #include <avr/power.h>
#endif
#include <fix_fft.h>

#define COUNT_OF(x) (sizeof((x)) / sizeof((0[x])))

const int NEOPIXELS_PIN = 2;
const int ONBOARD_LED = 1;
const int MICROPHONE_ANALOG_PIN = A0;
const int MODE_COUNT = 4;
const int SAMPLE_COUNT = 128;  // Must be a power of 2
const int SAMPLING_FREQUENCY_HZ = 9600;  // Must be less than 10000 due to ADC
const int UPDATES_PER_SECOND = 50;
const int PIXEL_RING_COUNT = 16;
const int MODE_TIME_MS = 8000;
const int MAX_EXPECTED_AUDIO = 20;  // Change this to vary sensitivity

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_RING_COUNT * 2, NEOPIXELS_PIN);

uint8_t mode = 0;  // Current animation effect
uint32_t color = 0xFF0000;  // Start red


void setup() {
#ifdef __AVR_ATtiny85__  // Trinket, Gemma, etc.
  if(F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin();
  pixels.setBrightness(20);

  Serial.begin(9600);
  clearLeds();

  analogReference(EXTERNAL);
  pinMode(NEOPIXELS_PIN, OUTPUT);
  pinMode(ONBOARD_LED, OUTPUT);
}


void spectrumAnalyzer() {
  clearLeds();
  static char samples[SAMPLE_COUNT];
  static char im[SAMPLE_COUNT];
  // I'm not sure why this is 8, but that's the example I was looking at
  static uint8_t sample_averages[8];

  for (int i = 0; i < SAMPLE_COUNT; ++i) {
    samples[i] = analogRead(MICROPHONE_ANALOG_PIN);
    im[i] = 0;
  }

  fix_fft(samples, im, COUNT_OF(sample_averages), 0);

  // Make the samples positive
  for (int i = 0; i < 64; ++i) {
    samples[i] = sqrt(samples[i] * samples[i] + im[i] * im[i]);
  }

  // Average the values
  for (int i = 0; i < static_cast<int>(COUNT_OF(sample_averages)); ++i) {
    int j = i * 8;
    sample_averages[i] = 0;
    for (int k = 0; k < static_cast<int>(COUNT_OF(sample_averages)); ++k) {
      sample_averages[i] += samples[j + k];
    }
    if (i == 0) {
      sample_averages[i] /= 2;
    }
    sample_averages[i] = map(sample_averages[i], 0, MAX_EXPECTED_AUDIO, 0, 2 * PIXEL_RING_COUNT - 1);
  }

  for (int i = 0; i < static_cast<int>(COUNT_OF(sample_averages)); ++i) {
    // TODO: Set a color, other than blue
    pixels.setPixelColor(i, sample_averages[i]);
  }
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
  pixels.show();
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
    if (((offset + i) & 0b111) < 2) {
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
  for (int i = 0; i < PIXEL_RING_COUNT * 2; i++) {
    pixels.setPixelColor(i, 0);
  }
}

void loop() {
  static uint32_t prevTime = millis();

  // I considered using an array of function pointers, but that took an extra 100 bytes or so
  switch(mode) {
    case 0:  // Random sparks - just one LED on at a time!
      randomSparks();
      break;

    case 1:
      binaryClock();
      break;
 
    case 2:  // Spinny wheels (8 LEDs on at a time)
      spinnyWheels();
      break;

    case 3:
      spectrumAnalyzer();
      break;

    default:
      break;
  }

  static uint8_t index = 0;
  // x = reduce(lambda a, b: a + b, ([i] * 3 for i in range(7))); import random; random.shuffle(x); print(x)
  const static uint8_t color_indexes[] = {2, 1, 0, 3, 0, 6, 5, 1, 4, 1, 0, 5, 3, 2, 6, 3, 4, 5, 2, 6, 4};
  const static uint32_t colors[] = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF, 0xFFFFFF};

  const uint32_t t = millis();
  if ((t - prevTime) > MODE_TIME_MS) {
    color = colors[color_indexes[index]];
    index = (index + 1) % (sizeof(color_indexes) / sizeof(color_indexes[0]));
    ++mode;
    if (mode >= MODE_COUNT) {
      mode = 0;
    }
    clearLeds();
    prevTime = t;
  }
}
