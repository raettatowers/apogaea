#include <Arduino.h>
#include <arduinoFFT.h>
#include <stdint.h>
#include <FastLED.h>

#include "constants.hpp"

static const int SAMPLE_COUNT = 1024;
static const float SAMPLING_FREQUENCY_HZ = 41000;
static const float MINIMUM_DIVISOR = 2000;
static const int STRAND_COUNT = 5;
static const int STRAND_LENGTH = 100;
static const int BUCKET_COUNT = 20;
static const int MINIMUM_THRESHOLD = 1;
// Generated from python3 steps.py 1024 41000
//constexpr uint8_t VREAL_TO_BUCKET[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 16, 17, 19, 21, 24, 26, 29};
constexpr uint8_t VREAL_TO_BUCKET[] = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 18, 20, 21, 23, 25, 28, 30, 33};
static_assert(COUNT_OF(VREAL_TO_BUCKET) == BUCKET_COUNT);
static_assert(VREAL_TO_BUCKET[COUNT_OF(VREAL_TO_BUCKET) - 1] < SAMPLE_COUNT / 2);

static FftType vReal[SAMPLE_COUNT];
static FftType vImaginary[SAMPLE_COUNT];
static arduinoFFT fft(vReal, vImaginary, SAMPLE_COUNT, SAMPLING_FREQUENCY_HZ);
static QueueHandle_t queue;

extern CRGB leds[STRIP_COUNT][LEDS_PER_STRIP];

void setupSpectrumAnalyzer();
void spectrumAnalyzer();

static void collectSamples();
static void computeFft();
static void logChanges();
static void logHighest();
static void renderFft();
static void runInThread(void *);
static void slideDown();

#define FOR_VREAL for (int i = 0; i < COUNT_OF(vReal) / 2; ++i)

static void computeFft() {
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
  // Bass lines have more energy than higher samples, so just reduce them a smidge
  for (int i = 0; i < COUNT_OF(VREAL_TO_BUCKET) / 2; ++i) {
    vReal[i] *= 0.75;
  }
}

static void renderFft() {
  FftType maxSample = -1;
  FOR_VREAL {
    maxSample = max(maxSample, vReal[i]);
  }
  maxSample = max(maxSample, static_cast<FftType>(MINIMUM_DIVISOR));
  // Map them all to 0.0 .. 1.0
  const FftType multiplier = 1.0 / maxSample;
  FOR_VREAL {
    vReal[i] = vReal[i] * multiplier;
  }

  uint8_t buckets[BUCKET_COUNT] = {0};
  // TODO: Because we skip some, e.g. bucket 25 doesn't correspond to a note, we could probably
  // average that into bucket 26 for more accuracy.
  int bucketIndex = 0;
  for (const auto vrIndex : VREAL_TO_BUCKET) {
    // Do 254 to avoid floating point problems
    buckets[bucketIndex] = vReal[vrIndex] * 254;
    ++bucketIndex;
  }

  //fadeOut();

  // Okay. So there are 5 strands that I'm going to loop down and back up. I want the bassline to be
  // on the ouside edge, going up, and the other notes to trickle down from the center.

  // Do treble first
  slideDown();
  // Then add the new one
  for (int i = 0; i < STRIP_COUNT * 2; ++i) {
    // Black
    auto color = CHSV(0, 0, 0);
    const auto value = buckets[i + BUCKET_COUNT / 2];
    if (value > MINIMUM_THRESHOLD) {
      color = CHSV(value, 255, value);
    }

    if (i % 2 == 0) {
      leds[i / 2][0] = color;
    } else {
      leds[i / 2][LEDS_PER_STRIP - 1] = color;
    }
  }

  // Then bass line
  for (int i = 0; i < STRIP_COUNT * 2; ++i) {
    // For testing, let's always just clear the bottom
    fill_solid(&leds[i / 2][LEDS_PER_STRIP / 2 - 10], 20, CRGB::Black);

    // Black
    auto color = CHSV(0, 0, 0);
    const auto value = buckets[i];
    if (value > MINIMUM_THRESHOLD) {
      // Red
      color = CHSV(0, 255, value);
    }

    // TODO: I might need to reverse these two?
    const int length = value / 16;
    if (i % 2 == 0) {
      fill_solid(&leds[i / 2][LEDS_PER_STRIP / 2], length, color);
    } else {
      fill_solid(&leds[i / 2][LEDS_PER_STRIP / 2 - length], length, color);
    }
  }
}

static void collectSamples() {
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

static void runInThread(void *) {
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

static void slideDown() {
  for (int i = 0; i < STRIP_COUNT; ++i) {
    for (int j = LEDS_PER_STRIP / 2 - 1; j >= 1; --j) {
      leds[i][j] = leds[i][j - 1];
    }
    for (int j = LEDS_PER_STRIP / 2 - 1; j < LEDS_PER_STRIP - 1; ++j) {
      leds[i][j] = leds[i][j + 1];
    }
  }
}

static int bucketToLed(const int bucket) {
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
static void logChanges() {
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
