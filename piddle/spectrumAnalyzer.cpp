#include <Arduino.h>
#include <arduinoFFT.h>
#include <stdint.h>
#include <FastLED.h>
#include <driver/i2s.h>

#include "constants.hpp"

extern void blink(const int delay_ms);

static const int I2S_SAMPLE_RATE_HZ = 44100; // Sample rate of the I2S microphone

static const int SAMPLE_COUNT = 2048;
static const int MINIMUM_THRESHOLD = 20;
// Generated from python3 steps.py 2048 44100
static constexpr uint16_t NOTE_TO_VREAL_INDEX[] = {
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16, 18, 20, 22, 24, 27, 30, 32, 36, 40, 45, 48, 54, 61, 64, 72, 81, 91, 97, 109, 122, 129, 145, 163, 183
};
static const int NOTE_COUNT = COUNT_OF(NOTE_TO_VREAL_INDEX);
static_assert(NOTE_TO_VREAL_INDEX[NOTE_COUNT - 1] < SAMPLE_COUNT / 2, "Too few samples to represent all notes");

// These values can be changed in RemoteXY
int startTrebleNote = c4Index;
float minimumDivisor = 1000;
int additionalTrebleRange = 0;

FftType vReal[SAMPLE_COUNT];
static FftType vImaginary[SAMPLE_COUNT];
static arduinoFFT fft(vReal, vImaginary, SAMPLE_COUNT, I2S_SAMPLE_RATE_HZ);

extern CRGB leds[STRIP_COUNT][LEDS_PER_STRIP];
// We'll save a copy of the LEDs every time we update, so that the bass disappears instantly instead
// of trickling down with the rest of the LEDs
CRGB ledsBackup[STRIP_COUNT][LEDS_PER_STRIP];

void setupSpectrumAnalyzer();
void spectrumAnalyzer();

static void collectSamples();
static void computeFft();
static void renderFft();
static void slideDown();
static FftType maxVRealForNote(int note);
static void normalizeSamplesTo0_1(FftType samples[], int length);
static void logNotes(const FftType noteValues[NOTE_COUNT]);

// TODO: Should this start at 2?
#define FOR_VREAL for (int i = 0; i < COUNT_OF(vReal) / 2; ++i)

static void computeFft() {
  // I tried all the windowing types with music and with a pure sine wave.
  // For music, HAMMING seemed to do best, but WELCH and TRIANGLE seem to work best for sine wave.
  fft.Windowing(vReal, SAMPLE_COUNT, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  fft.Compute(vReal, vImaginary, SAMPLE_COUNT, FFT_FORWARD);
  fft.ComplexToMagnitude(vReal, vImaginary, SAMPLE_COUNT);
  // Samples 0, 1, and SAMPLE_COUNT - 1 are the sample average or something, so just drop them
  vReal[0] = 0.0;
  vReal[1] = 0.0;
  vReal[SAMPLE_COUNT - 1] = 0.0;
  // Bass lines have more energy than higher samples, so just reduce them a smidge
  for (int i = 0; i < NOTE_TO_VREAL_INDEX[startTrebleNote]; ++i) {
    vReal[i] *= 0.5;
  }

#if false
  // Debug logging
  const int interval_ms = 5000;
  static decltype(millis()) nextDisplayTime_ms = interval_ms;
  if (millis() < nextDisplayTime_ms) {
    return;
  }
  nextDisplayTime_ms = millis() + interval_ms;
  for (int i = 0; i < COUNT_OF(vReal) / 2; ++i) {
    Serial.printf("%03d:%f\n", i, vReal[i]);
  }
#endif
}

static uint8_t hueOffset = 0;
static void renderFft() {
  // Normalize the bass and treble separately
  const int startTrebleVRealIndex = NOTE_TO_VREAL_INDEX[startTrebleNote];
  normalizeSamplesTo0_1(vReal, startTrebleVRealIndex);
  normalizeSamplesTo0_1(&vReal[startTrebleVRealIndex], SAMPLE_COUNT / 2 - startTrebleVRealIndex);

  // Okay. So there are 5 strands that I'm going to loop down and back up. I want the bassline to be
  // on the outside edge, going up, and the other notes to trickle down from the center.

  // First, restore the copy of the LEDs
  for (int i = 0; i < STRIP_COUNT; ++i) {
    memcpy(leds[i], ledsBackup[i], sizeof(leds[0]));
  }
  slideDown();

  FftType noteValues[NOTE_COUNT];
  for (int note = 0; note < NOTE_COUNT; ++note) {
    noteValues[note] = maxVRealForNote(note);
  }

  #if false
  FastLED.clear();
  for (int i = 0; i < COUNT_OF(noteValues); ++i) {
    leds[0][i] = CRGB(static_cast<int>(noteValues[i] * 255), 0, 0);
  }
  FastLED.show();
  logNotes(noteValues);
  #endif

  // Let's just do the top half of the strips at first
  int strip = 0;
  const int offset = c4Index - 7;
  for (int strip = 0; strip < STRIP_COUNT; ++strip) {
    const float red_f = noteValues[offset + 0 + strip];
    const float green_f = noteValues[offset + 7 + strip];
    const float blue_f = noteValues[offset + 14 * strip];
    const uint8_t red = static_cast<uint8_t>(red_f * 254);
    const uint8_t green = static_cast<uint8_t>(green_f * 254);
    const uint8_t blue = static_cast<uint8_t>(blue_f * 254);
    leds[strip][0] = CRGB(red, green, blue);
  }
  FastLED.show();
}

static void collectSamples() {
  static int16_t data[SAMPLE_COUNT];
  const int maxI2sSize = 1024 / sizeof(data[0]);
  const int usPerSample = 1000 * 1000 / I2S_SAMPLE_RATE_HZ;
  int16_t* ptr;

  size_t bytesRead;
  if (SAMPLE_COUNT <= maxI2sSize) {
    i2s_read(I2S_NUM_0, data, sizeof(data), &bytesRead, portMAX_DELAY);
  } else {
    ptr = &data[0];
    int size = SAMPLE_COUNT;
    while (size > maxI2sSize) {
      i2s_read(I2S_NUM_0, ptr, maxI2sSize * sizeof(data[0]), &bytesRead, portMAX_DELAY);
      const auto start_us = micros();
      size -= maxI2sSize;
      ptr += size;
      const auto diff_us = micros() - start_us;
      if (diff_us > usPerSample) {
        delayMicroseconds(usPerSample - diff_us);
      }
    }
    i2s_read(I2S_NUM_0, ptr, size * sizeof(data[0]), &bytesRead, portMAX_DELAY);
  }
  static_assert(COUNT_OF(data) == COUNT_OF(vReal));
  for (int i = 0; i < COUNT_OF(data); ++i) {
    vReal[i] = data[i];
  }

  // Reset vImaginary
  std::fill(vImaginary, vImaginary + COUNT_OF(vImaginary), 0.0);

#if false
  // Testing
  for (const auto sample: data) {
    Serial.println(sample);
  }
#endif
}

void spectrumAnalyzer() {
  collectSamples();
  computeFft();
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

void setupSpectrumAnalyzer() {
  const auto I2S_BITS_PER_SAMPLE = I2S_BITS_PER_SAMPLE_16BIT; // 16-bit audio samples
  const int SCK_PIN = 19;
  const int WS_PIN = 22;
  const int SD_PIN = 21;

  i2s_config_t i2sConfig = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),  // I2S receive mode
    .sample_rate = I2S_SAMPLE_RATE_HZ,
    .bits_per_sample = I2S_BITS_PER_SAMPLE,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Mono channel
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = 0, // Default interrupt allocation
    .dma_buf_count = 8, // Number of DMA buffers
    .dma_buf_len = SAMPLE_COUNT, // Size of each DMA buffer. TODO Not sure what this should be?
    .use_apll = false // Use the internal APLL (Audio PLL)
  };

  // Install and configure the I2S driver
  i2s_driver_install(I2S_NUM_0, &i2sConfig, 0, NULL);

  // Set pins for I2S
  i2s_pin_config_t pin_config = {
    .bck_io_num = SCK_PIN,
    .ws_io_num = WS_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = SD_PIN
  };
  i2s_set_pin(I2S_NUM_0, &pin_config);

  // Configure I2S input format
  i2s_set_clk(I2S_NUM_0, I2S_SAMPLE_RATE_HZ, I2S_BITS_PER_SAMPLE, I2S_CHANNEL_MONO);
}

/**
 * Returns the max vReal value in a particular note. For example, if NOTE_TO_VREAL_INDEX[c4Index] =
 * 39 and NOTE_TO_VREAL_INDEX[c4Index] = 43, it will look in vReal[39:43] and return the largest value.
 */
static FftType maxVRealForNote(const int note) {
  if (note >= COUNT_OF(NOTE_TO_VREAL_INDEX) - 1) {
    return vReal[COUNT_OF(NOTE_TO_VREAL_INDEX) - 1];
  }
  FftType maxVReal = vReal[NOTE_TO_VREAL_INDEX[note]];
  for (int i = NOTE_TO_VREAL_INDEX[note]; i < NOTE_TO_VREAL_INDEX[note + 1]; ++i) {
    maxVReal = max(maxVReal, vReal[i]);
  }
  return maxVReal;
}


/**
 * Normalize the samples to [0..1], or lower if all the samples are low
 */
static void normalizeSamplesTo0_1(FftType samples[], const int length) {
  FftType minSample = std::numeric_limits<FftType>::max();
  for (int i = 0; i < length; ++i) {
    minSample = min(minSample, samples[i]);
  }
  for (int i = 0; i < length; ++i) {
    samples[i] -= minSample;
  }
  FftType maxSample = std::numeric_limits<FftType>::min();
  for (int i = 0; i < length; ++i) {
    maxSample = max(maxSample, samples[i]);
  }
  // Always have some divisor, in case all the values are low
  maxSample = max(maxSample, static_cast<FftType>(minimumDivisor));
  // Map them all to 0.0 .. 1.0
  const FftType multiplier = 1.0 / maxSample;
  for (int i = 0; i < length; ++i) {
    samples[i] = samples[i] * multiplier;
  }
}

static void logNotes(const FftType noteValues[NOTE_COUNT]) {
  const int interval_ms = 5000;
  static decltype(millis()) nextDisplayTime_ms = interval_ms;
  if (millis() < nextDisplayTime_ms) {
    return;
  }
  nextDisplayTime_ms = millis() + interval_ms;

  for (int i = 0; i < NOTE_COUNT; ++i) {
    Serial.printf("%02d:%d\n", i, static_cast<int>(noteValues[i] * 255));
  }
  Serial.println();
}
