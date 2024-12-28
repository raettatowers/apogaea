#include <Arduino.h>
#include <arduinoFFT.h>
#include <stdint.h>
#include <FastLED.h>
#include <driver/i2s_std.h>
#include <esp_log.h>
#include <esp_check.h>
#include <assert.h>

#include "constants.hpp"

static const int I2S_SAMPLE_RATE_HZ = 44100; // Sample rate of the I2S microphone
static const int MAX_I2S_BUFFER_LENGTH = 512;

static const int SAMPLE_COUNT = 2048;
static const int MINIMUM_THRESHOLD = 20;
// Generated from python3 steps.py 2048 44100
static constexpr uint16_t NOTE_TO_VREAL_INDEX[] = {
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16, 18, 20, 22, 24, 27, 30, 32, 36, 40, 45, 48, 54, 61, 64, 72, 81, 91, 97, 109, 122, 129, 145, 163, 183
};
static const int NOTE_COUNT = COUNT_OF(NOTE_TO_VREAL_INDEX);
static_assert(NOTE_TO_VREAL_INDEX[NOTE_COUNT - 1] < SAMPLE_COUNT / 2, "Too few samples to represent all notes");

// Slide down twice to make it move faster (just 1 for developing)
const int SLIDE_COUNT = 1;

// This value used to be changeable in RemoteXY
float minimumDivisor = 1000;

static FftType vReal[SAMPLE_COUNT];
static FftType vImaginary[SAMPLE_COUNT];

// This probably doesn't need to be souble the SAMPLE_COUNT, SAMPLE_COUNT + MAX_I2S_BUFFER_LENGTH
// (or maybe + MAX_I2S_BUFFER_LENGTH * 2) would probably cut it. Ah well, this works. If I run low
// on memory, it's something to consider.
static int16_t rawSamples[SAMPLE_COUNT * 2];
static volatile int rawSamplesOffset = 0;
static arduinoFFT fft(vReal, vImaginary, SAMPLE_COUNT, I2S_SAMPLE_RATE_HZ);
static float weightingConstants[SAMPLE_COUNT];

static i2s_chan_handle_t rxHandle;

extern CRGB leds[STRIP_COUNT][LEDS_PER_STRIP];
extern bool logDebug;

static void computeFft();
static void renderFft();
static void slideDown(int count);
static FftType maxVRealForNote(int note);
static void normalizeSamplesTo0_1(FftType samples[], int length);
static void logNotes(const FftType noteValues[NOTE_COUNT]);
static float aWeightingMultiplier(const float frequency);

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

static void renderFft() {
  // Bass lines have more energy than higher samples, so reduce them
  for (int i = 0; i < COUNT_OF(vReal); ++i) {
    vReal[i] *= weightingConstants[i];
  }

  normalizeSamplesTo0_1(vReal, SAMPLE_COUNT);

  // Okay. So there are 5 strands that I'm going to loop down and back up. I want the bassline to be
  // on the outside edge, going up, and the other notes to trickle down from the center.

  // First, restore the copy of the LEDs
  // I was initially doing this when I wanted to do the bass line at the bottom, but not doing it for now
  //for (int i = 0; i < STRIP_COUNT; ++i) {
  //  memcpy(leds[i], ledsBackup[i], sizeof(leds[0]));
  //}
  // Slide down more than once to make it move faster (just 1 for developing)
  slideDown(SLIDE_COUNT);

  FftType noteValues[NOTE_COUNT];
  for (int note = 0; note < NOTE_COUNT; ++note) {
    noteValues[note] = maxVRealForNote(note);
  }
  if (logDebug) {
    FastLED.clear();
    for (int i = 0; i < COUNT_OF(noteValues); ++i) {
      leds[0][i] = CRGB(static_cast<int>(noteValues[i] * 255), 0, 0);
    }
    FastLED.show();
    logNotes(noteValues);
    logDebug = false;
  }

  const int offset = c4Index;  // I used to have c4Index - 7 here
  for (int physical = 0; physical < STRIP_COUNT; ++physical) {
    // Top half
    {
      const float red_f = noteValues[offset + 0 + physical * 2];
      const float green_f = noteValues[offset + 7 + physical * 2];
      const float blue_f = noteValues[offset + 14 + physical * 2];
      const uint8_t red = static_cast<uint8_t>(red_f * 254);
      const uint8_t green = static_cast<uint8_t>(green_f * 254);
      const uint8_t blue = static_cast<uint8_t>(blue_f * 254);
      for (int i = 0; i < SLIDE_COUNT; ++i) {
        leds[physical][i] = CRGB(red, green, blue);
      }
    }
    // Bottom half
    {
      const float red_f = noteValues[offset + 0 + physical * 2 + 1];
      const float green_f = noteValues[offset + 7 + physical * 2 + 1];
      const float blue_f = noteValues[offset + 14 + physical * 2 + 1];
      const uint8_t red = static_cast<uint8_t>(red_f * 254);
      const uint8_t green = static_cast<uint8_t>(green_f * 254);
      const uint8_t blue = static_cast<uint8_t>(blue_f * 254);
      for (int i = 0; i < SLIDE_COUNT; ++i) {
        leds[physical][LEDS_PER_STRIP - 1 - i] = CRGB(red, green, blue);
      }
    }
  }
}

void collectSamples() {
  // This algorithm won't work if either of these are false
  static_assert(SAMPLE_COUNT > MAX_I2S_BUFFER_LENGTH);
  static_assert(SAMPLE_COUNT % MAX_I2S_BUFFER_LENGTH == 0);

  const int usPerSample = 1000 * 1000 / I2S_SAMPLE_RATE_HZ;
  // We're just going to continually read into rawSamples
  size_t bytesRead;
  auto start_us = micros();
  auto diff_us = micros();
  while (1) {
    const int oldOffset = rawSamplesOffset;
    // Mark this section as unable to be used
    rawSamplesOffset += MAX_I2S_BUFFER_LENGTH;
    if (rawSamplesOffset >= COUNT_OF(rawSamples)) {
      rawSamplesOffset = 0;
    }

    diff_us = micros() - start_us;
    if (diff_us < usPerSample) {
      delayMicroseconds(usPerSample - diff_us);
    }
    start_us = micros();

    // TODO: Could use i2s_channel_register_event_callback() and change the delay to 0 to make this
    // async. Then use this core to do more processing
    ESP_ERROR_CHECK(
      i2s_channel_read(
        rxHandle,
        &rawSamples[oldOffset],
        MAX_I2S_BUFFER_LENGTH * sizeof(rawSamples[0]),
        &bytesRead,
        portMAX_DELAY
      )
    );
    assert(bytesRead == MAX_I2S_BUFFER_LENGTH * sizeof(rawSamples[0]));
  }
}

void displaySpectrumAnalyzer() {
  static auto start_ms = millis();
  const decltype(millis()) logTime_ms = 10000;
  static int loopCount = 0;

  auto part_ms = millis();
  // First we need to copy the data from the samples circular buffer
  // We can't use memcpy because we're converting uint16_t to float
  const int sampleOffset = rawSamplesOffset;
  if (sampleOffset + COUNT_OF(vReal) < COUNT_OF(rawSamples)) {
    for (int i = 0; i < COUNT_OF(vReal); ++i) {
      vReal[i] = rawSamples[sampleOffset + i];
    }
  } else {
    // Let's say length = 10, offset = 13
    // Then I need to copy 7 items (2*length-offset) starting at 13
    const int upper = COUNT_OF(rawSamples) - sampleOffset;
    for (int i = 0; i < upper; ++i) {
      vReal[i] = rawSamples[sampleOffset + i];
    }
    // Then copy the last 3 items
    const int lower = COUNT_OF(vReal) - upper;
    for (int i = 0; i < lower; ++i) {
      vReal[i + upper] = rawSamples[i];
    }
  }

  // Reset vImaginary
  std::fill(vImaginary, vImaginary + COUNT_OF(vImaginary), 0.0);
  const auto samples_ms = millis() - part_ms;

  part_ms = millis();
  computeFft();
  const auto compute_ms = millis() - part_ms;

  part_ms = millis();
  renderFft();
  const auto render_ms = millis() - part_ms;

  part_ms = millis();
  FastLED.show();
  const auto show_ms = millis() - part_ms;

  ++loopCount;
  if (start_ms + logTime_ms < millis()) {
    Serial.printf("%f FPS\n", static_cast<double>(loopCount) * 1000 / logTime_ms);
    Serial.printf(
      "samples_ms:%ld compute_ms:%ld render_ms:%ld show_ms:%ld\n",
      samples_ms,
      compute_ms,
      render_ms,
      show_ms
    );
    start_ms = millis();
    loopCount = 0;
  }

}

static void slideDown(const int count) {
  const int byteCount = (LEDS_PER_STRIP / 2 - count) * sizeof(leds[0][0]);
  for (int i = 0; i < STRIP_COUNT; ++i) {
    memmove(
      &leds[i][count],
      &leds[i][0],
      byteCount
    );
    memmove(
      &leds[i][LEDS_PER_STRIP / 2],
      &leds[i][LEDS_PER_STRIP / 2 + count],
      byteCount
    );
  }
}

void setupSpectrumAnalyzer() {
  i2s_chan_config_t channelConfig = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
  // The above macro sets channelConfig to {
  //  .id = I2S_NUM_AUTO,
  //  .role = I2S_ROLE_MASTER,
  //  .dma_desc_num = 6,
  //  .auto_clear_after_cb = 0,
  //  .auto_clear_before_cb = 0,
  //  .intr_priority = 0,
  // }

  // Send nullptr for the tx handle, we're only receiving
  ESP_ERROR_CHECK(i2s_new_channel(&channelConfig, nullptr, &rxHandle));

  i2s_std_config_t stdConfig = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE_HZ),
    // Slot mode only matters when transmitting
    .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
    .gpio_cfg = {
      .mclk = I2S_GPIO_UNUSED,
      .bclk = GPIO_NUM_19,
      .ws = GPIO_NUM_22,
      .dout = I2S_GPIO_UNUSED,
      .din = GPIO_NUM_21,
      .invert_flags = {
        .mclk_inv = false,
        .bclk_inv = false,
        .ws_inv = false,
      },
    },
  };
  ESP_ERROR_CHECK(i2s_channel_init_std_mode(rxHandle, &stdConfig));

  ESP_ERROR_CHECK(i2s_channel_enable(rxHandle));

  // Bass notes have higher percieved energy, because the human ear is weird. To compensate, we'll
  // do A weighting.
  for (int i = 0; i < SAMPLE_COUNT; ++i) {
    const float freq = static_cast<float>(I2S_SAMPLE_RATE_HZ) / SAMPLE_COUNT * (i + 1);
    weightingConstants[i] = aWeightingMultiplier(freq);
  }

  std::fill(rawSamples, rawSamples + COUNT_OF(rawSamples), 0);
}

constexpr float square(const float f) {
  return f * f;
}

/**
 * Returns the A weighting multiplier for a frequency, see https://en.wikipedia.org/wiki/A-weighting
 */
static float aWeightingMultiplier(const float frequency) {
  const float freq_2 = square(frequency);
  const float denom1 = freq_2 + square(20.6f);
  const float denom2 = sqrtf((freq_2 + square(107.7f)) * (freq_2 + square(737.9f)));
  const float denom3 = freq_2 + square(12194.0f);
  const float denom = denom1 * denom2 * denom3;
  const float enumer = (square(freq_2) * square(12194.0f));
  const float ra = enumer / denom;
  const float aWeighting_db = 2.0f + 20.0f * logf(ra) / logf(10.0f);
  const float multiplier = powf(10.0f, aWeighting_db / 10.0f);
  return multiplier;
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
  for (int i = 0; i < NOTE_COUNT; ++i) {
    Serial.printf("%02d:%d\n", i, static_cast<int>(noteValues[i] * 255));
  }
  Serial.println();
}
