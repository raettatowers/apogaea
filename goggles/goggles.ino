#include <Adafruit_NeoPixel.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
 #include <avr/power.h>
#endif
#include <fix_fft.h>

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

// On Uno, if you're using Serial, this needs to be > 1. On Trinket it should be 0.
const int NEOPIXELS_PIN = 2;
const int ONBOARD_LED = 1;
const int MICROPHONE_ANALOG_PIN = A0;
const int MODE_COUNT = 4;
const int SAMPLE_COUNT = 128;  // Must be a power of 2
const int PIXEL_RING_COUNT = 16;
const int MODE_TIME_MS = 8000;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_RING_COUNT * 2, NEOPIXELS_PIN);

uint16_t hue = 0;  // Start red
uint32_t color = pixels.ColorHSV(hue);


void setup() {
#ifdef __AVR_ATtiny85__  // Trinket, Gemma, etc.
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin();
  pixels.setBrightness(20);

  Serial.begin(9600);
  clearLeds();

#ifndef ADAFRUIT_TRINKET_M0
  analogReference(DEFAULT);
#else
  // TODO: Figure out what to do on Trinket M0
#endif
  pinMode(MICROPHONE_ANALOG_PIN, INPUT);
  pinMode(NEOPIXELS_PIN, OUTPUT);
  pinMode(ONBOARD_LED, OUTPUT);
}


template <int A, int B>
struct getPower
{
  static const int value = A * getPower <A, B - 1>::value;
};
template <int A>
struct getPower<A, 0>
{
  static const int value = 1;
};
void spectrumAnalyzer() {
  clearLeds();
  static int8_t samples[SAMPLE_COUNT];
  static int8_t imaginary[SAMPLE_COUNT];
  static int8_t sampleAverages[2 * PIXEL_RING_COUNT];

  for (uint16_t i = 0; i < COUNT_OF(samples); ++i) {
    samples[i] = analogRead(MICROPHONE_ANALOG_PIN) * 3;
    imaginary[i] = 0;
  }

  // TODO: Can do fix_fftr if I'm not using imaginary numbers
  const int16_t power = 7;
  static_assert(getPower<2, power>::value == COUNT_OF(samples), "");
  fix_fft(samples, imaginary, power, 0);

  // Make values positive, only the first half of the ouput represents usable frequency values
  for (uint16_t i = 0; i < COUNT_OF(samples) / 2; ++i) {
    samples[i] = sqrt(samples[i] * samples[i] + imaginary[i] * imaginary[i]);
  }

  static_assert(COUNT_OF(samples) % COUNT_OF(sampleAverages) == 0, "");
  // Only the first half of the output has usable frequency values
  const int usableValues = COUNT_OF(samples) / 2;
  const int stepSize = usableValues / COUNT_OF(sampleAverages);

  // The first sample is the average power of the signal, so skip it
  int counter = 1;
  // Use a sentinel value to make this loop simpler, since we start at 1
  samples[usableValues] = 0;
  for (uint16_t i = 0; i < COUNT_OF(sampleAverages); ++i) {
    sampleAverages[i] = 0;
    for (int j = 0; j < stepSize; ++j, ++counter) {
      sampleAverages[i] += samples[counter];
    }
    // This division should compile down to a right bit shift
    sampleAverages[i] /= stepSize;
  }

  // Multiply by 4 just to get better response
  // TODO: Improve this visualization
  for (uint16_t i = 0; i < COUNT_OF(sampleAverages); ++i) {
    pixels.setPixelColor(i, sampleAverages[i] * 4);
  }
  pixels.show();
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
  static uint8_t offset = 0;  // Position of spinny eyes
  for (uint8_t i = 0; i < PIXEL_RING_COUNT; ++i) {
    uint32_t c = 0;
    if (((offset + i) & 0b111) < 2) {
      c = color;  // 4 pixels on...
    }
    pixels.setPixelColor(i, c);  // First eye
    pixels.setPixelColor(PIXEL_RING_COUNT * 2 - i, c);  // Second eye (flipped)
  }
  pixels.show();
  ++offset;
  hue += 20;  // Make the rainbow colors rotate faster
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
  for (int i = 0; i < PIXEL_RING_COUNT * 2; ++i) {
    pixels.setPixelColor(i, 0);
  }
}


void loop() {
  static uint32_t modeStartTime_ms = millis();
  static uint8_t mode = 0;  // Current animation effect
  static void (*animations[])() = {spinnyWheels, binaryClock, randomSparks, spectrumAnalyzer};

  const uint32_t startAnimation_ms = millis();
  animations[mode]();

  const uint32_t now_ms = millis();
  // We want to complete a full hue color cycle about every 10 seconds
  const int hueCycle_ms = 10000;
  const int hueCycleLimit = 65535;
  hue += (startAnimation_ms - now_ms) * (hueCycleLimit / hueCycle_ms);
  color = pixels.ColorHSV(hue);

  if ((now_ms - modeStartTime_ms) > MODE_TIME_MS) {
    ++mode;
    if (mode > COUNT_OF(animations)) {
      mode = 0;
    }
    clearLeds();
    modeStartTime_ms = now_ms;
  }
}
