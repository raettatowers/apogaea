#include <Arduino.h>
#include <arduinoFFT.h>
#include <stdint.h>
#include <FastLED.h>

#include "constants.hpp"

static const int SAMPLE_COUNT = 256;
static const float SAMPLING_FREQUENCY_HZ = 5000;
static const float MINIMUM_DIVISOR = 1000;
static const int STRAND_COUNT = 5;
static const int BUCKET_COUNT = 16;
// Run python3 steps.py BUCKET_COUNT SAMPLE_COUNT/2 to get this
static constexpr int STEPS[] = {27, 22, 17, 14, 11, 9, 7, 5, 4, 3, 3, 2, 1, 1, 1, 1};
static_assert(COUNT_OF(STEPS) == BUCKET_COUNT);
// sum(STEPS) should equal the SAMPLE_COUNT / 2
template <int N, int _unused> struct sum { static const int value = sum<N - 1, 0>::value + STEPS[N];};
template <int _unused> struct sum<0, _unused> { static const int value = STEPS[0]; };
static_assert(sum<COUNT_OF(STEPS) - 1, 0>::value == SAMPLE_COUNT / 2);

static FftType vReal[SAMPLE_COUNT];
static FftType vImaginary[SAMPLE_COUNT];
static arduinoFFT fft(vReal, vImaginary, SAMPLE_COUNT, SAMPLING_FREQUENCY_HZ);
static QueueHandle_t queue;

extern CRGB leds[LED_COUNT];

void collectSamples();
void computeFft();
void logChanges();
void logHighest();
void renderFft();
void runInThread(void *);
void setupSpectrumAnalyzer();
void spectrumAnalyzer();

#define FOR_VREAL for (int i = 0; i < COUNT_OF(vReal); ++i)

void computeFft() {
  // I tried all the windowing types with music and with a pure sine wave.
  // For music, FFT_WIN_TYP_HAMMING seemed to do best, but FFT_WIN_TYP_TRIANGLE
  // seemed best for the sine wave. HANN and TRIANGLE also did well.
  fft.Windowing(vReal, SAMPLE_COUNT, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  fft.Compute(vReal, vImaginary, SAMPLE_COUNT, FFT_FORWARD);
  fft.ComplexToMagnitude(vReal, vImaginary, SAMPLE_COUNT);
  // Samples 0, 1, and SAMPLE_COUNT - 1 are the sample average or something, so just drop them
  vReal[0] = 0.0;
  vReal[1] = 0.0;
  vReal[SAMPLE_COUNT - 1] = 0.0;
}

void renderFft() {
  FftType maxSample = -1;
  maxSample = max(maxSample, static_cast<FftType>(MINIMUM_DIVISOR));
  // Map them all to 0.0 .. 1.0
  const FftType multiplier = 1.0 / maxSample;
  FOR_VREAL {
    vReal[i] = vReal[i] * multiplier;
  }

  FftType averages[BUCKET_COUNT];
  int realIndex = 0;
  int avgIndex = 0;
  for (const auto step : STEPS) {
    averages[avgIndex] = 0;
    for (int i = 0; i < step; ++i) {
      averages[avgIndex] += vReal[realIndex];
      ++realIndex;
    }
    averages[avgIndex] /= step;
    // Do 254 to avoid any floating point issues
    averages[avgIndex] *= 254;
    ++avgIndex;
  }

  // Add a fade off effect
  static uint8_t peaks[BUCKET_COUNT] = {0};
  static_assert(COUNT_OF(averages) == COUNT_OF(peaks));
  for (int i = 0; i < COUNT_OF(averages); ++i) {
    if (peaks[i] > 3) {
      // If you want to do peaks, uncomment this
      //peaks[i] -= 2;
      // Otherwise, no drop off
      peaks[i] = 0;
    }
    if (averages[i] > peaks[i]) {
      peaks[i] = averages[i];
    }

    leds[i] = CHSV(0, 255, peaks[i]);
  }

  // // Just for testing
  // static uint8_t hue = 0;
  // leds[0] = CHSV(hue, 100, 100);
  // ++hue;
}

void collectSamples() {
  static const uint32_t samplingPeriod_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY_HZ));
  static_assert(sizeof(micros()) == 4);
  // micros() returns an unsigned 4 byte number, so we can only run for 4.3B / 1M = 4.3K seconds or
  // 71 minutes. We definitely need to worry about overflow.

  const auto start = micros();
  for (int i = 0; i < SAMPLE_COUNT; ++i) {
    vReal[i] = analogRead(MICROPHONE_ANALOG_PIN);
    vImaginary[i] = 0;
    // This should handle overflow, or at least not lock up
    const auto limit = start + i * samplingPeriod_us;
    while (micros() < limit) {
      // Do nothing
    }
  }
}

void runInThread(void *) {
  int item = 0;
  for (;;) {
    if (uxQueueMessagesWaiting(queue) > 0) {
      computeFft();
    }
    // Release handle
    xQueueReceive(queue, &item, 0);
    renderFft();
  }
}

void spectrumAnalyzer() {
  collectSamples();
  computeFft();
  renderFft();
  //logChanges() ; delay(1000);
  //logHighest(); delay(1000);
}

int bucketToLed(const int bucket) {
  // With SAMPLE_COUNT = 512 and SAMPLING_FREQUENCY_HZ = 5000, I get:
  // C4 = 27
  // C4.5 = 28
  // D4 = 30
  // D4.5 = 32
  // E4 = 34
  // F4 = 36
  // F4.5 = 38
  // G4 = 40
  // G4.5 = 43
  // A4 = 45
  // A4.5 = 48
  // B4 = 51
  // C5 = 54
  // C5.5 = 57
  // D5 = 60
  // D5.5 = 64
  // E5 = 68
  // F5 = 71
  // F5.5 = 76
  // G5 = 80
  // G5.5 = 85
  // A5 = 90
  // A5.5 = 95
  // B5 = 101

  return -1;  
}

/**
 * Just for testing, log any time the highest index changes
 */
void logChanges() {
  FftType maxSample = -1;
  FOR_VREAL {
    maxSample = max(maxSample, vReal[i]);
  }
  // Set a default max so that if it's quiet, we're not visualizing random noises
  maxSample = max(maxSample, static_cast<FftType>(MINIMUM_DIVISOR));
  const FftType multiplier = 1.0 / maxSample;
  // Clamp them all to 0.0 - 1.0
  for (auto& vr : vReal) {
    vr *= multiplier;
  }

  Serial.print(".");
  // Skip the first couple samples because they're just the average, and bass line
  static int previousHighestIndex = 0;
  int highestIndex = 0;
  FftType highest = vReal[2];
  // The first 2? samples are the average power of the signal, so skip them.
  // Samples < 20Hz are so low that humans can't hear them, so skip those too.
  for (int i = 10; i < SAMPLE_COUNT / 2; ++i) {
    if (vReal[i] > highest) {
      highest = vReal[i];
      highestIndex = i;
    }
  }
  if (previousHighestIndex != highestIndex) {
    Serial.printf("\nNew highest: %d, value %f pre-scaled:%f\n", highestIndex, vReal[highestIndex], vReal[highestIndex] / multiplier);
    Serial.printf("Max: %f\n", maxSample);
  }
  previousHighestIndex = highestIndex;
}

/**
 * Just for testing, log the highest value.
 */
void logHighest() {
  Serial.print(".");
  // Skip the first couple samples because they're just the average, and bass line
  int highestIndex = 0;
  FftType highest = vReal[10];
  for (int i = 2; i < 90; ++i) {
    if (vReal[i] > highest) {
      highest = vReal[i];
      highestIndex = i;
    }
  }
  Serial.printf("highest before normalization index: %d, value: %f\n", highestIndex, highest);
}

void setupSpectrumAnalyzer() {
  queue = xQueueCreate(1, sizeof(int));
  assert(queue != nullptr);
}
