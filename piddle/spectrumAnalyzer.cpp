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
static const int MINIMUM_THRESHOLD = 1;
// Generated from python3 steps.py 2048 41000
constexpr uint8_t VREAL_TO_BUCKET[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 16, 17, 19, 21, 24, 26, 29, 32, 34, 39, 43, 49, 52, 58};
static const int BUCKET_COUNT = COUNT_OF(VREAL_TO_BUCKET);
static_assert(VREAL_TO_BUCKET[COUNT_OF(VREAL_TO_BUCKET) - 1] < SAMPLE_COUNT / 2);

static FftType _vReal[SAMPLE_COUNT];
static FftType _samples[SAMPLE_COUNT];
static FftType* vReal = _vReal;
static FftType* samples = _samples;
static FftType vImaginary[SAMPLE_COUNT];
static arduinoFFT fft(vReal, vImaginary, SAMPLE_COUNT, SAMPLING_FREQUENCY_HZ);
static QueueHandle_t queue;

extern CRGB leds[STRIP_COUNT][LEDS_PER_STRIP];

void setupSpectrumAnalyzer(const int arduinoCore);
void spectrumAnalyzer();

static void collectSamples();
static void computeFft();
static void renderFft();
static void runInThread(void *);
static void slideDown();

#define FOR_SAMPLES for (int i = 0; i < COUNT_OF(samples) / 2; ++i)

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
  // TODO: Alternatively, we could just normalize the bass and treble samples separately
  for (int i = 0; i < COUNT_OF(VREAL_TO_BUCKET) / 2; ++i) {
    vReal[i] *= 0.75;
  }
}

static void renderFft() {
  FftType maxSample = -1;
  FOR_SAMPLES {
    maxSample = max(maxSample, samples[i]);
  }
  maxSample = max(maxSample, static_cast<FftType>(MINIMUM_DIVISOR));
  // Map them all to 0.0 .. 1.0
  const FftType multiplier = 1.0 / maxSample;
  FOR_SAMPLES {
    samples[i] = samples[i] * multiplier;
  }

  uint8_t buckets[BUCKET_COUNT] = {0};
  // TODO: Because we skip some, e.g. bucket 25 doesn't correspond to a note, we could probably
  // average that into bucket 26 for more accuracy.
  int bucketIndex = 0;
  for (const auto vrIndex : VREAL_TO_BUCKET) {
    // Do 254 to avoid floating point problems
    buckets[bucketIndex] = samples[vrIndex] * 254;
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
      // TODO: More colors. FOr testing, let's just do blue.
      color = CHSV(160, 255, value);
    }

    if (i % 2 == 0) {
      leds[i / 2][0] = color;
    } else {
      leds[i / 2][LEDS_PER_STRIP - 1] = color;
    }
  }

  return;
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

    const int length = value / 16;
    // TODO: I might need to reverse these two?
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
    samples[i] = analogRead(MICROPHONE_ANALOG_PIN);
    // This should handle overflow, or at least not lock up
    const auto limit = start + i * samplingPeriod_us;
    while (micros() < limit) {
      // Busy wait
    }
  }

  // Wait for the other core to finish processing
  while (uxQueueMessagesWaiting(queue) != 0) {
    // Busy wait
  }

  // Once it's finished processing, we want to swap the samples and the recently computed FFT
  std::swap(samples, vReal);
  // and reset vImaginary
  std::fill(vImaginary, vImaginary + COUNT_OF(vImaginary), 0.0);

  // Notify the other core
  int item = 0;
  xQueueSend(queue, &item, portMAX_DELAY);
}

static void runInThread(void *) {
  int unused = 0;
  while (true) {
    if (uxQueueMessagesWaiting(queue) > 0) {
      computeFft();
      // Release handle
      xQueueReceive(queue, &unused, 0);
    }
  }
}

void spectrumAnalyzer() {
  collectSamples();
  renderFft();
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

void setupSpectrumAnalyzer(const int arduinoCore) {
  queue = xQueueCreate(1, sizeof(int));
  assert(queue != nullptr);

  const int core = arduinoCore == 0 ? 1 : 0;
  xTaskCreatePinnedToCore(
    runInThread,
    "computeFft", // Name of the task (for debugging)
    10000, // Stack size (bytes)
    NULL, // Parameter to pass
    1, // Task priority
    NULL, // Task handle
    core // Core you want to run the task on (0 or 1)
  );
}
