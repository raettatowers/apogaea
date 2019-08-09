// Arduino Beat Detector originally Damian Peckett 2015
// License: Public Domain.

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#include "constants.hpp"

// Our Global Sample Rate, 5000hz
const int SAMPLE_PERIOD_US = 200;

typedef int32_t beatLevel_t;
// The most recent beat is stored in previousBeats[0]
static beatLevel_t previousBeats[2];
static const beatLevel_t THRESHOLD = 5;


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
static beatLevel_t getBeat() {
  unsigned long time = micros(); // Used to track rate
  float sample, value, envelope;
  static uint8_t brightnessIndex;

  // Every 200 samples (25hz) filter the envelope
  for (int i = 0; i < 200; ++i) {
    // Read ADC and center so +-512
    sample = static_cast<float>(analogRead(MICROPHONE_ANALOG_PIN)) - 503.f;

    // Filter only bass component
    value = bassFilter(sample);

    // Take signal amplitude and filter
    if (value < 0) {
      value = -value;
    }
    envelope = envelopeFilter(value);

    // Consume excess clock cycles, to keep at 5000 hz
    for (unsigned long up = time + SAMPLE_PERIOD_US; time > 20 && time < up; time = micros());
  }

  // Filter out repeating bass sounds 100 - 180bpm
  return static_cast<beatLevel_t>(beatFilter(envelope));
}


static void recordBeat() {
  for (int i = COUNT_OF(previousBeats) - 1; i > 0; --i) {
    previousBeats[i] = previousBeats[i - 1];
  }
  previousBeats[0] = getBeat();
}


static bool beatDetected() {
  return previousBeats[0] >= THRESHOLD && previousBeats[1] >= THRESHOLD;
}


void flashLensesToBeat(Adafruit_NeoPixel* const pixels, const uint16_t hue) {
  // Flash the left lens, then the right
  static uint8_t lens = 0;
  static uint8_t brightness = 0;
  const uint8_t MAX_BRIGHTNESS = 50;
  const uint8_t DROP_OFF = 10;

  recordBeat();

  extern bool reset;
  if (reset) {
    brightness = 0;
  }

  if (brightness > 0) {
    if (brightness > DROP_OFF) {
      brightness -= DROP_OFF;
    } else {
      brightness = 0;
      // Shut off that lens
      pixels->fill(0, lens * PIXEL_RING_COUNT, PIXEL_RING_COUNT);
      // Toggle lens
      lens ^= 1;
    }
  } else if (beatDetected()) {
    brightness = MAX_BRIGHTNESS;
  }

  const uint32_t color = pixels->ColorHSV(hue, 0xFF, brightness);
  pixels->fill(color, lens * PIXEL_RING_COUNT, PIXEL_RING_COUNT);
  pixels->show();
}


void rotateGearsToBeat(Adafruit_NeoPixel* const pixels, const uint16_t hue) {
  static uint8_t start = 0;
  static uint32_t lastBeatMillis = 0;
  const uint8_t SKIP = 4;
  static_assert(PIXEL_RING_COUNT % SKIP == 0, "SKIP value gives ugly gears");
  const uint8_t BRIGHTNESS = 50;

  recordBeat();

  if (beatDetected() && millis() - lastBeatMillis > 200) {
    ++start;
    if (start == SKIP) {
      start = 0;
    }
    lastBeatMillis = millis();
  }

  pixels->fill(0, 0, PIXEL_RING_COUNT * 2);
  const uint32_t color = pixels->ColorHSV(hue, 0xFF, BRIGHTNESS);
  for (int i = start; i < PIXEL_RING_COUNT; i += SKIP) {
    pixels->setPixelColor(i, color);
  }
  // The second lens moves the opposite direction
  for (int i = PIXEL_RING_COUNT * 2 - start; i >= PIXEL_RING_COUNT; i -= SKIP) {
    pixels->setPixelColor(i, color);
  }
  pixels->show();
}
