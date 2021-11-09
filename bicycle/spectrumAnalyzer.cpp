#include <Arduino.h>
#include <arduinoFFT.h>
#include <stdint.h>
#include <Adafruit_DotStar.h>

#include "animations.hpp"
#include "constants.hpp"

// Note: I manually edited the arduinoFFT files so that it uses floats instead
// of doubles, so that it's faster. You'll need to do that too. Alternatively,
// you can change this back to double if you don't want to edit library files.
// With the double version, I get about 13 updates per second; with the float
// version, I get 66.
typedef double FftType;

// Sample count must be a power of 2. I chose 128 because only half of the values
// from the FFT correspond to frequencies, and the first 2 are sample averages, so
// I needed more than 2 * PIXEL_RING_COUNT * 2 + 2, which gives me 128.
static const int SAMPLE_COUNT = 128;

SpectrumAnalyzer::SpectrumAnalyzer(Adafruit_DotStar &leds_, uint8_t numLeds_) :
  leds(leds_),
  numLeds(numLeds_)
{
}

void SpectrumAnalyzer::animate(ColorFunctor&) {
  // Most songs have notes in the lower end, so from experimental
  // observation, this seems like a good choice
  const uint32_t SAMPLING_FREQUENCY_HZ = 5000;
  const uint32_t samplingPeriod_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY_HZ));
  static arduinoFFT FFT = arduinoFFT();
  static uint8_t previousValues[LED_COUNT] = {0};
  // Change the base hue of low intensity sounds so we can get more colors
  static uint32_t baseHue = 0;

  FftType vReal[SAMPLE_COUNT];
  FftType vImaginary[SAMPLE_COUNT];
  FftType sampleAverages[LED_COUNT];
  const int usableValues = COUNT_OF(vReal) / 2;
  const int stepSize = usableValues / COUNT_OF(sampleAverages);

  baseHue += 100;

  for (int i = 0; i < SAMPLE_COUNT; ++i) {
    // TODO: Do we need to worry about overflow?
    const auto before = micros();

    vReal[i] = CircuitPlayground.mic.soundPressureLevel(10);
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

  FftType max_ = -1;
  for (auto sa : sampleAverages) {
    max_ = max(max_, sa);
  }
  // Set a default max so that if it's quiet, we're not visualizing random noises
  max_ = max(max_, 400);
  const FftType MULTIPLIER = 1.0 / max_;
  // Clamp them all to 0.0 - 1.0
  for (auto& sa : sampleAverages) {
    sa *= MULTIPLIER;
  }

  const uint8_t MAX_BRIGHTNESS = 128;
  // This cutoff needs some tweaking based on pixels->getBrightness()
  // 10 works with brightness = 20
  const uint8_t CUTOFF = 10;

  for (uint8_t i = 0; i < COUNT_OF(sampleAverages); ++i) {
    // Implement a fade-off effect
    const int FADE_OFF = 20;
    const uint8_t value_ = max(previousValues[i] > FADE_OFF ? previousValues[i] - FADE_OFF : 0, sampleAverages[i] * 0xFF);
    previousValues[i] = value_;
    const uint32_t hue = static_cast<uint32_t>(value_) * 0xFF + baseHue;
    uint8_t brightness = min(MAX_BRIGHTNESS, value_);
    // The visualization looks like garbage when brightness is high because all of the
    // pixels turn on, and I want dim pixels off
    if (brightness < CUTOFF) {
      brightness = 0;
    }
    leds.setPixelColor(i, leds.ColorHSV(hue * 255, 0xFF, brightness));
  }
}
