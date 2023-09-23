#include <Arduino.h>
#include <arduinoFFT.h>
#include <stdint.h>
#include <FastLED.h>

#include "constants.hpp"

static const int SAMPLE_COUNT = 512;
static const float SAMPLING_FREQUENCY_HZ = 41000 / 8;
static const float MINIMUM_DIVISOR = 4000;
static const int STRAND_COUNT = 5;
static const int STRAND_LENGTH = 100;
static const int MINIMUM_THRESHOLD = 20;
// Generated from python3 steps.py 512 5125
static constexpr uint16_t NOTE_TO_VREAL_INDEX[] = {4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 16, 17, 19, 21, 24, 26, 29, 32, 34, 39, 43, 49, 52, 58, 65, 69, 78, 87, 98, 104, 117, 131, 139, 156, 175, 197, 209, 234};
const int c4Index = 15;
const int startTrebleIndex = 20;
static const int NOTE_COUNT = COUNT_OF(NOTE_TO_VREAL_INDEX);
static_assert(NOTE_TO_VREAL_INDEX[NOTE_COUNT - 1] < SAMPLE_COUNT / 2, "Too few samples to represent all notes");

static FftType vReal[SAMPLE_COUNT];
static FftType vImaginary[SAMPLE_COUNT];
static arduinoFFT fft(vReal, vImaginary, SAMPLE_COUNT, SAMPLING_FREQUENCY_HZ);

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
static void normalizeSamplesTo0_1();
static void logNotes(const FftType noteValues[NOTE_COUNT]);

// TODO: Should this start at 2?
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
  for (int i = 0; i < NOTE_TO_VREAL_INDEX[startTrebleIndex]; ++i) {
    vReal[i] *= 0.75;
  }
}

static uint8_t hueOffset = 0;
static void renderFft() {
  normalizeSamplesTo0_1();

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

  logNotes(noteValues);

  // Do treble first
  for (int i = 0; i < STRAND_COUNT; ++i) {
    // Let's do max of 2 notes
    FftType maxValueOf2 = max(noteValues[startTrebleIndex + i], noteValues[startTrebleIndex + i + STRAND_COUNT]);

    // 254 to avoid rounding problems
    const uint8_t value = maxValueOf2 * 254;
    auto color = CHSV(0, 0, 0);  // Black
    if (value > MINIMUM_THRESHOLD) {
      const uint8_t hue = hueOffset + i * (256 / (STRIP_COUNT * 2));
      color = CHSV(hue, 255, value);
    }

    if (i % 2 == 0) {
      leds[i / 2][0] = color;
    } else {
      leds[i / 2][LEDS_PER_STRIP - 1] = color;
    }
  }
  ++hueOffset;

  // Then bass line
  // But first, save the copy of the LEDs
  for (int i = 0; i < STRIP_COUNT; ++i) {
    memcpy(ledsBackup[i], leds[i], sizeof(leds[0]));
  }

  static_assert(startTrebleIndex - STRIP_COUNT * 2 > 2);
  const auto bassColor = CHSV(0, 255, 128);  // Dim red
  for (int i = 0; i < STRIP_COUNT * 2; ++i) {
    const auto value = NOTE_TO_VREAL_INDEX[startTrebleIndex + i - STRIP_COUNT * 2];
    if (value < MINIMUM_THRESHOLD) {
      continue;
    }

    const int length = min(value / 4, LEDS_PER_STRIP);
    // TODO: I might need to reverse these two?
    if (i % 2 == 0) {
      fill_solid(&leds[i / 2][LEDS_PER_STRIP / 2], length, bassColor);
    } else {
      fill_solid(&leds[i / 2][LEDS_PER_STRIP / 2 - length], length, bassColor);
    }
  }
}

static void collectSamples() {
  static const uint32_t samplingPeriod_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY_HZ));
  static_assert(sizeof(micros()) == 4);
  // micros() returns an unsigned 4 byte number, so we can only run for 4.3B / 1M = 4.3K seconds or
  // 71 minutes. We definitely need to worry about overflow.

  for (int i = 0; i < SAMPLE_COUNT; ++i) {
    const auto start_us = micros();
    vReal[i] = analogRead(MICROPHONE_ANALOG_PIN);
    // This should handle overflow, or at least not lock up
    const auto limit_us = start_us + samplingPeriod_us;
    while (micros() < limit_us) {
      // Busy wait
    }
  }

  // and reset vImaginary
  std::fill(vImaginary, vImaginary + COUNT_OF(vImaginary), 0.0);
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
 * Normalize all the samples to [0..1], or lower if all the samples are low
 */
static void normalizeSamplesTo0_1() {
  // TODO: Normalize the bass and treble separately?
  FftType minSample = std::numeric_limits<FftType>::max();
  FOR_VREAL {
    minSample = min(minSample, vReal[i]);
  }
  FOR_VREAL {
    vReal[i] -= minSample;
  }
  FftType maxSample = std::numeric_limits<FftType>::min();
  FOR_VREAL {
    maxSample = max(maxSample, vReal[i]);
  }
  // Always have some divisor, in case all the values are low
  maxSample = max(maxSample, static_cast<FftType>(MINIMUM_DIVISOR));
  // Map them all to 0.0 .. 1.0
  const FftType multiplier = 1.0 / maxSample;
  FOR_VREAL {
    vReal[i] = vReal[i] * multiplier;
  }
}

static void logNotes(const FftType noteValues[NOTE_COUNT]) {
  static decltype(millis()) nextDisplayTime = 1000;
  if (millis() < nextDisplayTime) {
    return;
  }
  nextDisplayTime = millis() + 1000;

  for (int i = 0; i < NOTE_COUNT; ++i) {
    Serial.printf("%02d:", i);
    for (int j = 0; j < static_cast<int>(noteValues[i] * 32); ++j) {
      Serial.print("-");
    }
    Serial.println();
  }
  Serial.println();
}
