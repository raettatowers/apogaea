#include <Arduino.h>
#include <arduinoFFT.h>
#include <stdint.h>
#include <FastLED.h>

#include "constants.hpp"

static const int SAMPLE_COUNT = 512;
static const float SAMPLING_FREQUENCY_HZ = 44100;
static const float AMPLITUDE = 200.0;
static const int NUM_BANDS = 8;

static FftType vReal[SAMPLE_COUNT];
static FftType vImaginary[SAMPLE_COUNT];
static FftType sampleAverages[SAMPLE_COUNT];
static arduinoFFT fft(vReal, vImaginary, SAMPLE_COUNT, SAMPLING_FREQUENCY_HZ);
static QueueHandle_t queue;
static int32_t peak[] = {0, 0, 0, 0, 0, 0, 0, 0};

extern CRGB leds[LED_COUNT];

static int visualizationCounter = 0;
static int32_t lastVisualizationUpdate = 0;

void setupSpectrumAnalyzer() {
  queue = xQueueCreate(1, sizeof(int));
  assert(queue != nullptr);
}

static void createBands(int i, int dsize) {
  uint8_t band = 0;
  if (i <= 2) {
    band = 0; // 125Hz
  } else if (i <= 5) {
    band = 1; // 250Hz
  } else if (i <= 7) {
    band = 2; // 500Hz
  } else if (i <= 15) {
    band = 3; // 1000Hz
  } else if (i <= 30) {
    band = 4; // 2000Hz
  } else if (i <= 53) {
    band = 5; // 4000Hz
  } else if (i <= 106) {
    band = 6; // 8000Hz
  } else {
    band = 7;
  }
  int dmax = AMPLITUDE;
  if (dsize > dmax) {
    dsize = dmax;
  }
  if (dsize > peak[band]) {
    peak[band] = dsize;
  }
}

void computeFft() {
  // I tried all the windowing types with music and with a pure sine wave.
  // For music, FFT_WIN_TYP_HAMMING seemed to do best, but FFT_WIN_TYP_TRIANGLE
  // seemed best for the sine wave. HANN and TRIANGLE also did well.
  fft.Windowing(vReal, SAMPLE_COUNT, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  fft.Compute(vReal, vImaginary, SAMPLE_COUNT, FFT_FORWARD);
  fft.ComplexToMagnitude(vReal, vImaginary, SAMPLE_COUNT);

  // The first 2 samples? are the average power of the signal, so skip them.
  // Samples < 20Hz are so low that humans can't hear them, so skip those too.
  const int SKIP = 2 + 5;
  const auto averagePower = vReal[0];
}

void renderFft() {
  static uint8_t previousValues[LED_COUNT] = {0};
  // Change the base hue of low intensity sounds so we can get more colors
  static uint8_t baseHue = 0;

  baseHue += 1;

  FftType (&sampleAverages)[LED_COUNT] = collectSamples();

  // The bassline really drowns out the other samples, so reduce its effect
  // Actually, is that because the FFT has very small buckets for lower samples?
  // If the average intensity of half an octave is stuffed into a single bucket,
  // that might be causing it to be overrepresented. TODO
  for (uint8_t i = 0; i < COUNT_OF(sampleAverages) / 8; ++i) {
    sampleAverages[i] *= 0.5;
  }
  for (uint8_t i = 0; i < COUNT_OF(sampleAverages) / 4; ++i) {
    sampleAverages[i] *= 0.5;
  }

  FftType maxSample = -1;
  for (auto sa : sampleAverages) {
    maxSample = max(maxSample, sa);
  }
  // Set a default max so that if it's quiet, we're not visualizing random noises
  maxSample = max(maxSample, 1000);
  const FftType MULTIPLIER = 1.0 / maxSample;
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

  /*
  if ((millis() - lastVisualizationUpdate) > 1000) {
    log_e("Fps: %f", visualizationCounter / ((millis() - lastVisualizationUpdate) / 1000.0));
    visualizationCounter = 0;
    lastVisualizationUpdate = millis();
    hueOffset += 5;
  }
  */
  visualizationCounter++;
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
}
