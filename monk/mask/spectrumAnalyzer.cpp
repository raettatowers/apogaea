#include <Arduino.h>
#include <arduinoFFT.h>
#include <cstdint>
#include <FastLED.h>

#include "constants.hpp"

// Note: I manually edited the arduinoFFT files so that it uses floats instead
// of doubles, so that it's faster. These 2 lines just make the type match. If
// you want, you can edit the library to use floats, or just leave it. This
// should compile either way.
static arduinoFFT FFT = arduinoFFT();
typedef decltype(FFT.MajorPeak()) FftType;

// Sample count must be a power of 2. I chose 128 because only half of the values
// from the FFT correspond to frequencies, and the first 2 are sample averages, so
// I needed more than LED_COUNT * 2 + 2, which gives me 128.
static const int SAMPLE_COUNT = 128;
static_assert(LED_COUNT * 2 + 2 < SAMPLE_COUNT, "");
static_assert(LED_COUNT * 2 + 2 >= SAMPLE_COUNT / 2, "");

extern CRGB leds[LED_COUNT];


// collectSamples returns a reference to an array of FftType with size LED_SIZE
static FftType (&collectSamples())[LED_COUNT];


void spectrumAnalyzer(uint8_t) {

  static uint8_t previousValues[LED_COUNT] = {0};
  // Change the base hue of low intensity sounds so we can get more colors
  static uint8_t baseHue = 0;

  baseHue += 1;

  FftType (&sampleAverages)[LED_COUNT] = collectSamples();

  FftType max_ = -1;
  for (auto sa : sampleAverages) {
    max_ = max(max_, sa);
  }
  // Set a default max so that if it's quiet, we're not visualizing random noises
  max_ = max(max_, 1000);
  const FftType MULTIPLIER = 1.0 / max_;
  // Clamp them all to 0.0 - 1.0
  for (auto& sa : sampleAverages) {
    sa *= MULTIPLIER;
  }

  const uint8_t MAX_BRIGHTNESS = 128;
  // This cutoff needs some tweaking based on leds->getBrightness()
  // 10 works with brightness = 20
  const uint8_t CUTOFF = 10;

  for (uint8_t i = 0; i < COUNT_OF(sampleAverages); ++i) {
    // Implement a fade-off effect
    const int FADE_OFF = 20;
    uint8_t value = max(previousValues[i] > FADE_OFF ? previousValues[i] - FADE_OFF : 0, sampleAverages[i] * 0xFF);
    previousValues[i] = value;
    // Let's also reduce the color space a bit so that the colors aren't all over the place
    const uint8_t hue = value / 2 + baseHue;
    uint8_t brightness = min(MAX_BRIGHTNESS, value);
    // The visualization looks like garbage when brightness is high because all of the
    // LEDs turn on, and I want dim LEDs off
    if (brightness < CUTOFF) {
      brightness = 0;
    }
    leds[i] = CHSV(hue, 0xFF, brightness);
  }

  FastLED.show();
}


FftType (&collectSamples())[LED_COUNT] {
  // Most songs have notes in the lower end, so from experimental
  // observation, this seems like a good choice
  static const uint32_t SAMPLING_FREQUENCY_HZ = 5000;
  static const uint32_t samplingPeriod_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY_HZ));

  // Declare these static so that we're not reallocating every call, and so that we
  // can return a reference to sampleAverages
  static FftType vReal[SAMPLE_COUNT];
  static FftType vImaginary[SAMPLE_COUNT];
  static FftType sampleAverages[LED_COUNT];
  static const int usableValues = COUNT_OF(vReal) / 2;
  static const int stepSize = usableValues / COUNT_OF(sampleAverages);

  for (int i = 0; i < SAMPLE_COUNT; ++i) {
    // TODO: Do we need to worry about overflow?
    const auto before = micros();

    vReal[i] = analogRead(MICROPHONE_ANALOG_PIN);
    vImaginary[i] = 0;

    const auto now = micros();
    // I don't know if this is better than a busy wait loop or not
    delayMicroseconds(before + samplingPeriod_us - now);
  }

  FFT.Windowing(vReal, SAMPLE_COUNT, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImaginary, SAMPLE_COUNT, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImaginary, SAMPLE_COUNT);

  // The first 2 samples? are the average power of the signal, so skip them.
  // Samples < 20Hz are so low that humans can't hear them, so skip those too.
  const int SKIP = 2 + 5;
  uint8_t counter = SKIP;
  for (uint16_t i = 0; i < COUNT_OF(sampleAverages); ++i) {
    sampleAverages[i] = 0;
    // The FFT gives us equally spaced buckets (e.g. bucket 1 is 0-100 Hz, bucket 2 is 100-200 Hz)
    // but notes increase in frequency quadratically. In fact, each increase in octave is double
    // the frequency. As a kludge, just skip every other sample for the first half of the samples.
    const int increment = i < COUNT_OF(sampleAverages) / 2 ? 2 : 1;
    for (int j = 0; j < stepSize; j += increment, ++counter) {
      sampleAverages[i] += vReal[counter];
    }
    // This division should compile down to a right bit shift
    sampleAverages[i] /= stepSize;
    // To be accurate, we should multiply by the increment. However, I've found that percussion
    // tends to be lower, and it overwhelms the other stuff. Since we're biasing against low sounds
    // anyway, we just won't do that. I think the visualization looks better. Sorry bass line.
  }

  return sampleAverages;
}
