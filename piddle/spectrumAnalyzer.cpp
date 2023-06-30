#include <Arduino.h>
#include <arduinoFFT.h>
#include <stdint.h>
#include <FastLED.h>

#include "constants.hpp"

static const int SAMPLE_COUNT = 256;
static const float SAMPLING_FREQUENCY_HZ = 10000;
static const float MINIMUM_DIVISOR = 1000;
static const int STRAND_COUNT = 5;
static const int BUCKET_COUNT = 16;
// Put a value between 10 and 80. Smaller the number, higher the audio response
static const int MAX_AUDIO_RESPONSE = 20;

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
  static int count = 0;
  ++count;
  if (count > 255) {
    count = 0;
    Serial.println();
    Serial.println(millis());
  }

  FftType maxSample = -1;
  FOR_VREAL {
    maxSample = max(maxSample, vReal[i]);
  }
  if (count == 0) {
    Serial.printf("max:%0.1f\n", maxSample);
  }
  maxSample = max(maxSample, static_cast<FftType>(MINIMUM_DIVISOR));
  // Map them all to 0.0 .. 1.0
  const FftType multiplier = 1.0 / maxSample;
  if (count == 0) {
    Serial.printf("maxSample:%0.1f\n", maxSample);
    Serial.printf("vReal[5] before:%0.1f\n", vReal[5]);
  }
  FOR_VREAL {
    vReal[i] = vReal[i] * multiplier;
  }
  if (count == 0) {
    Serial.printf("vReal[5] after:%0.3f\n", vReal[5]);
  }

  const int step = SAMPLE_COUNT / 2 / BUCKET_COUNT;
  FftType averages[BUCKET_COUNT];
  for (int i = 0, avgIndex = 0; i < SAMPLE_COUNT / 2; i += step, ++avgIndex) {
    averages[avgIndex] = 0;
    for (int j = 0; j < step; j++) {
      averages[avgIndex] += vReal[i + j];
    }
    averages[avgIndex] /= step;
    // Do 254 to avoid any floating point issues
    averages[avgIndex] *= 254;
  }

  static uint8_t peaks[BUCKET_COUNT] = {0};
  static_assert(COUNT_OF(averages) == COUNT_OF(peaks));
  for (int i = 0; i < COUNT_OF(averages); ++i) {
    if (peaks[i] > 0) {
      --peaks[i];
    }
    if (averages[i] > peaks[i]) {
      peaks[i] = averages[i];
    }

    leds[i] = CHSV(0, 255, peaks[i]);
  }

  if (count == 0) {
    Serial.printf("peak[1]:%d\n", peaks[1]);
    Serial.printf("averages[1]:%0.1f\n", averages[1]);
    FftType sum = 0;
    for (int i = 0; i < step; ++i) {
      Serial.printf("%0.1f ", vReal[step + i]);
      sum += vReal[step + i];
    }
    Serial.printf("\nsum: %f\n", sum);
  }

  // Just for testing
  static uint8_t hue = 0;
  leds[0] = CHSV(hue, 100, 100);
  ++hue;
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
