// Arduino Beat Detector originally Damian Peckett 2015
// License: Public Domain.

#include <Arduino.h>
#include <FastLED.h>

#include "constants.hpp"

typedef int16_t beatLevel_t;

// Our global sample rate, 5000hz
static const int SAMPLE_RATE_HZ = 5000;
static const int SAMPLE_PERIOD_US = 1000000 / SAMPLE_RATE_HZ;
static const int FILTER_SAMPLES = 200;
static const beatLevel_t THRESHOLD = 5;
// BPM == 5000 / 200 * 60 / peaks
const int MINIMUM_PEAK_INTERVAL = 9;  // 166.67 BPM
const int MAXIMUM_PEAK_INTERVAL = 18;  // 83.33 BPM

// 20 - 200 hz Single Pole Bandpass IIR Filter
static float bassFilter(const float sample) {
  static float xv[3] = {0, 0, 0}, yv[3] = {0, 0, 0};
  xv[0] = xv[1]; xv[1] = xv[2];
  xv[2] = (sample) / 3.f; // change here to values close to 2, to adapt for stronger or weeker sources of line level audio

  yv[0] = yv[1]; yv[1] = yv[2];
  yv[2] = (xv[2] - xv[0]) + (-0.7960060012f * yv[0]) + (1.7903124146f * yv[1]);
  return yv[2];
}


// 10 hz Single Pole Lowpass IIR Filter
static float envelopeFilter(const float sample) {
  // 10 hz low pass
  static float xv[2] = {0, 0}, yv[2] = {0, 0};
  xv[0] = xv[1];
  xv[1] = sample / 50.f;
  yv[0] = yv[1];
  yv[1] = (xv[0] + xv[1]) + (0.9875119299f * yv[0]);
  return yv[1];
}


// 1.7 - 3.0hz Single Pole Bandpass IIR Filter
static float beatFilter(const float sample) {
  static float xv[3] = {0, 0, 0}, yv[3] = {0, 0, 0};
  xv[0] = xv[1]; xv[1] = xv[2];
  xv[2] = sample / 2.7f;
  yv[0] = yv[1]; yv[1] = yv[2];
  yv[2] = (xv[2] - xv[0]) + (-0.7169861741f * yv[0]) + (1.4453653501f * yv[1]);
  return yv[2];
}


// TODO: Do I want to convert this to an int, or just return a float?
beatLevel_t getBeat() {
  auto time = micros(); // Used to track rate
  float sample, value, envelope;

  // Every 200 samples (25hz) filter the envelope
  for (int i = 0; i < FILTER_SAMPLES; ++i) {
    // Read ADC and center so +-512
    sample = static_cast<float>(analogRead(MICROPHONE_ANALOG_PIN)) - 503.f;

    // Filter only bass component
    value = bassFilter(sample);

    // Take signal amplitude and filter
    if (value < 0) {
      value = -value;
    }
    envelope = envelopeFilter(value);

    // Consume excess clock cycles, to keep at SAMPLE_RATE_HZ hz
    for (auto up = time + SAMPLE_PERIOD_US; time > 20 && time < up; time = micros());
  }

  // Filter out repeating bass sounds 100 - 180bpm
  return static_cast<beatLevel_t>(beatFilter(envelope));
}

// TODO: Change these to static
beatLevel_t previousBeat;
bool increasing = false;
bool _beatDetected(const beatLevel_t currentBeat) {
  const int MISSES_BEFORE_RESET = 5;
  // 107 BPM is a peak interval of 107.14
  const int DEFAULT_PEAK_INTERVAL = 14;

  static uint8_t peakInterval = DEFAULT_PEAK_INTERVAL;
  static uint8_t samplesSinceLastPeak = MINIMUM_PEAK_INTERVAL;
  static uint8_t missedPeaks = MISSES_BEFORE_RESET;
  static bool lookingForNextPeak = false;

  bool debug = false;
  const auto now = millis();
  static uint32_t debugTime = 0;
  if (Serial.available() > 0) {
    debugTime = now;
    while (Serial.available() > 0) {
      Serial.read();
    }
    Serial.println("\n\n\n\n");
  }
  if (debugTime + 2000 > now) {
    debug = true;
  }

  if (debug) {
    Serial.print(now);
    Serial.print(" prev:");
    Serial.print(previousBeat, DEC);
    Serial.print(" curr:");
    Serial.print(currentBeat, DEC);
    Serial.print(" missed:");
    Serial.print(missedPeaks, DEC);
    Serial.print(" interval:");
    Serial.print(peakInterval, DEC);
    Serial.print(" sslp:");
    Serial.print(samplesSinceLastPeak, DEC);
    Serial.print(" inc:");
    Serial.print(increasing, DEC);
    Serial.println();
  }

  ++samplesSinceLastPeak;

  if (missedPeaks >= MISSES_BEFORE_RESET) {
    // Missed too many peaks - start over
    if (increasing && currentBeat < previousBeat && currentBeat >= THRESHOLD) {
      // Found a peak!
      if (lookingForNextPeak) {
        // Great, now we have an interval
        peakInterval = samplesSinceLastPeak;
        lookingForNextPeak = false;
        samplesSinceLastPeak = 0;
        missedPeaks = 0;
        if (debug) { Serial.print("Found 2 peaks, interval = "); Serial.println(peakInterval, DEC); }
        return true;
      } else if (MINIMUM_PEAK_INTERVAL <= samplesSinceLastPeak && samplesSinceLastPeak <= MAXIMUM_PEAK_INTERVAL) {
        // Found our first peak
        peakInterval = samplesSinceLastPeak;
        lookingForNextPeak = true;
        samplesSinceLastPeak = 0;
        if (debug) { Serial.println("Found first peak"); }
        return true;
      }
    } else if (lookingForNextPeak && samplesSinceLastPeak > MAXIMUM_PEAK_INTERVAL) {
      // Couldn't find another peak, reset
      lookingForNextPeak = false;
      samplesSinceLastPeak = 0;
      if (debug) { Serial.println("No second peak found, resetting"); }
    }
    return false;
  }

  // We only check for peaks when we are in peakInterval += 2
  if (peakInterval - 2 <= samplesSinceLastPeak && samplesSinceLastPeak <= peakInterval + 3) {
    if (increasing && currentBeat < previousBeat) {
      // Found a peak! Adjust the intervals.
      missedPeaks = 0;
      if (peakInterval < samplesSinceLastPeak) {
        ++peakInterval;
        if (debug) { Serial.print("++peakInterval = "); Serial.println(peakInterval); }
      } else if (peakInterval > samplesSinceLastPeak) {
        --peakInterval;
        if (debug) { Serial.print("--peakInterval = "); Serial.println(peakInterval); }
      } else {
        if (debug) { Serial.println("Exact peakInterval"); }
      }
      samplesSinceLastPeak = 1;
      return true;
    }
    return false;
  } else if (samplesSinceLastPeak > peakInterval + 3) {
    // I guess we missed it
    if (debug) { Serial.println("Missed it?"); }
    samplesSinceLastPeak = 4;
  }

  // If we miss a peak, just pretend we found one
  if (samplesSinceLastPeak == peakInterval) {
    if (debug) { Serial.println("Faking it"); }
    ++missedPeaks;
    return true;
  }
  return false;
}
static bool beatDetected() {
  static uint16_t beatsSinceLastDetected = 0;

  const beatLevel_t currentBeat = getBeat();
  const bool detected = _beatDetected(currentBeat);
  if (currentBeat > previousBeat) {
    increasing = true;
  } else {
    increasing = false;
  }
  previousBeat = currentBeat;

  // _beatDetected will optimistically return true when the predicted interval has
  // elapsed, even if a new peak hasn't been found. If a new peak is discovered soon
  // after, it will return true again. Avoid double counting.
  if (detected && beatsSinceLastDetected >= MINIMUM_PEAK_INTERVAL) {
    beatsSinceLastDetected = 0;
    return true;
  }
  ++beatsSinceLastDetected;
  return false;
}


void flashLensesToBeat(CRGB pixels[], const uint16_t hue) {
  // Flash the left lens, then the right
  static uint8_t lens = 0;
  static uint8_t brightness = 0;
  const uint8_t MAX_BRIGHTNESS = 50;
  const uint8_t DROP_OFF = 10;

  extern bool reset;
  if (reset) {
    brightness = 0;
  }

  // This has to be outside of the if statement, so that we still record beats
  const bool detected = beatDetected();
  if (detected) {
    Serial.println(millis());
  }
  if (brightness > 0) {
    if (brightness > DROP_OFF) {
      brightness -= DROP_OFF;
    } else {
      brightness = 0;
      // Shut off that lens
      fill_solid(&pixels[lens * PIXEL_RING_COUNT], PIXEL_RING_COUNT, CRGB::Black);
      // Toggle lens
      lens ^= 1;
    }
  } else if (detected) {
    brightness = MAX_BRIGHTNESS;
  }

  const auto color = CHSV(hue, 0xFF, brightness);
  fill_solid(&pixels[lens * PIXEL_RING_COUNT], PIXEL_RING_COUNT, color);
  FastLED.show();
}


void rotateGearsToBeat(CRGB pixels[], const uint16_t hue) {
  static uint8_t start = 0;
  static uint32_t lastBeatMillis = 0;
  const uint8_t SKIP = 4;
  static_assert(PIXEL_RING_COUNT % SKIP == 0, "SKIP value gives ugly gears");
  const uint8_t BRIGHTNESS = 50;

  if (beatDetected() && millis() - lastBeatMillis > 200) {
    ++start;
    if (start == SKIP) {
      start = 0;
    }
    lastBeatMillis = millis();
  }

  fill_solid(&pixels[0], PIXEL_RING_COUNT * 2, CRGB::Black);
  const auto color = CHSV(hue, 0xFF, BRIGHTNESS);
  for (int i = start; i < PIXEL_RING_COUNT; i += SKIP) {
    pixels[i] = color;
  }
  // The second lens moves the opposite direction
  for (int i = PIXEL_RING_COUNT * 2 - start; i >= PIXEL_RING_COUNT; i -= SKIP) {
    pixels[i] = color;
  }
  FastLED.show();
}
