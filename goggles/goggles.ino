#include <Adafruit_NeoPixel.h>
#include <Adafruit_DotStar.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
#include <avr/power.h>
#endif
#include <fix_fft.h>

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

// On Uno, if you're using Serial, this needs to be > 1. On Trinket it should be 0.
const int NEOPIXELS_PIN = 0;
const int ONBOARD_LED_PIN = 13;
const int MICROPHONE_ANALOG_PIN = A0;
const int MODE_COUNT = 4;
const int SAMPLE_COUNT = 128;  // Must be a power of 2
const int PIXEL_RING_COUNT = 16;
const int MODE_TIME_MS = 8000;

Adafruit_DotStar internalPixel = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_RING_COUNT * 2, NEOPIXELS_PIN);

uint16_t hue = 0;  // Start red
uint32_t color = pixels.ColorHSV(hue);


void setup() {
#ifdef __AVR_ATtiny85__  // Trinket, Gemma, etc.
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin();
  pixels.setBrightness(20);
  clearLeds();

  // Just shut off the internal LED
  internalPixel.setPixelColor(0, 0);
  internalPixel.show();

#ifndef ADAFRUIT_TRINKET_M0
  analogReference(DEFAULT);
#else
  // TODO: Figure out what to do on Trinket M0
#endif
  pinMode(MICROPHONE_ANALOG_PIN, INPUT);
  pinMode(NEOPIXELS_PIN, OUTPUT);
  pinMode(ONBOARD_LED_PIN, OUTPUT);
}


template <int A, int B>
struct getPower
{
  static const int value = A * getPower < A, B - 1 >::value;
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
  const uint8_t led1 = random(16);
  pixels.setPixelColor(led1, color);
  const uint8_t led2 = random(16) + PIXEL_RING_COUNT;
  pixels.setPixelColor(led2, color);
  pixels.show();
  delay(20);
  pixels.setPixelColor(led1, 0);
  pixels.setPixelColor(led2, 0);
  pixels.show();
}


void fadingSparks() {
  // Like random sparks, but they fade in and out
  static bool increasing[2 * PIXEL_RING_COUNT] = {
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
  };
  static int8_t brightnessIndexes[2 * PIXEL_RING_COUNT] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
  };
  // Let's keep them the same color that they started with
  static uint16_t hues[2 * PIXEL_RING_COUNT] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
  };
  // Let's use our own sparkHue so that we can change the pixels more quickly
  static uint16_t sparkHue = 0;
  // Start with some zeroes so that we don't relight a pixel immediately
  const static uint8_t brightnesses[] = {0, 0, 0, 0, 0, 0, 4, 8, 16, 32, 48, 64, 64, 64};

  // So we pick a random LED to start making brighter
  const uint8_t led = random(2 * PIXEL_RING_COUNT);
  if (!increasing[led]) {
    increasing[led] = true;
    hues[led] = sparkHue;
    sparkHue += 1200;
  }

  for (uint8_t i = 0; i < COUNT_OF(increasing); ++i) {
    if (increasing[i]) {
      if (brightnessIndexes[i] < static_cast<int>(COUNT_OF(brightnesses))) {
        ++brightnessIndexes[i];
      } else {
        increasing[i] = false;
      }
      pixels.setPixelColor(i, pixels.ColorHSV(hues[i], 0xFF, brightnesses[brightnessIndexes[i]]));
    } else {
      if (brightnessIndexes[i] > 0) {
        --brightnessIndexes[i];
        pixels.setPixelColor(i, pixels.ColorHSV(hues[i], 0xFF, brightnesses[brightnessIndexes[i]]));
      }
    }
  }
  delay(50);
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
    pixels.setPixelColor(PIXEL_RING_COUNT * 2 - 1 - i, c);  // Second eye (flipped)
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


void swirls() {
  static uint8_t head1 = 0;
  static uint8_t head2 = 3;
  static const uint8_t brightness[] = {255, 128, 64, 32, 16, 8, 4, 2, 1};

  clearLeds();
  // I could probably optimize this loop so that I'm not going through the whole ring
  for (uint8_t i = 0; i < PIXEL_RING_COUNT; ++i) {
    const int8_t difference1 = head1 - i;
    if (0 <= difference1 && difference1 < static_cast<int>(COUNT_OF(brightness))) {
      pixels.setPixelColor(i, pixels.ColorHSV(hue, 0xFF, brightness[difference1]));
    } else {
      const int8_t difference2 = PIXEL_RING_COUNT + difference1;
      if (0 <= difference2 && difference2 < static_cast<int>(COUNT_OF(brightness))) {
        pixels.setPixelColor(i, pixels.ColorHSV(hue, 0xFF, brightness[difference2]));
      }
    }
  }
  head1 = (head1 + 1) % PIXEL_RING_COUNT;

  // Make the second lens swirl the other direction
  for (uint8_t i = 0; i < PIXEL_RING_COUNT; ++i) {
    const int8_t difference1 = head2 - i;
    if (0 <= difference1 && difference1 < static_cast<int>(COUNT_OF(brightness))) {
      pixels.setPixelColor(PIXEL_RING_COUNT * 2 - 1 - i, pixels.ColorHSV(hue, 0xFF, brightness[difference1]));
    } else {
      const int8_t difference2 = PIXEL_RING_COUNT + difference1;
      if (0 <= difference2 && difference2 < static_cast<int>(COUNT_OF(brightness))) {
        pixels.setPixelColor(PIXEL_RING_COUNT * 2 - 1 - i, pixels.ColorHSV(hue, 0xFF, brightness[difference2]));
      }
    }
  }
  head2 = (head2 + 1) % PIXEL_RING_COUNT;

  pixels.show();
  delay(50);
}


void shimmer() {
  // Start above 0 so that each light should be on a bit
  static const uint8_t brightness[] = {4, 8, 16, 32, 64, 128};
  static uint8_t values[PIXEL_RING_COUNT * 2] = {
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
  };

  for (int i = 0; i < 4; ++i) {
    const uint8_t pixel = random(PIXEL_RING_COUNT * 2);
    if (random(2) == 1) {
      if (values[pixel] < COUNT_OF(brightness)) {
        ++values[pixel];
      } else {
        --values[pixel];
      }
    } else {
      if (values[pixel] > 0) {
        --values[pixel];
      } else {
        ++values[pixel];
      }
    }
  }

  for (int i = 0; i < PIXEL_RING_COUNT * 2; ++i) {
    pixels.setPixelColor(i, pixels.ColorHSV(hue, 0xFF, brightness[values[i]]));
  }
  pixels.show();
  delay(50);
}


void clearLeds() {
  for (int i = 0; i < PIXEL_RING_COUNT * 2; ++i) {
    pixels.setPixelColor(i, 0);
  }
}


void loop() {
  static uint32_t modeStartTime_ms = millis();
  static uint8_t mode = 0;  // Current animation effect
  static void (*animations[])() = {spinnyWheels, binaryClock, fadingSparks, spectrumAnalyzer, shimmer, swirls};

  const uint32_t startAnimation_ms = millis();
  animations[mode]();

  const uint32_t now_ms = millis();
  // We want to complete a full hue color cycle about every X seconds
  const int hueCycle_ms = 20000;
  const int hueCycleLimit = 65535;
  hue += (startAnimation_ms - now_ms) * (hueCycleLimit / hueCycle_ms);
  color = pixels.ColorHSV(hue);

  if ((now_ms - modeStartTime_ms) > MODE_TIME_MS) {
    ++mode;
    if (mode >= COUNT_OF(animations)) {
      mode = 0;
    }
    clearLeds();
    modeStartTime_ms = now_ms;
  }
}
