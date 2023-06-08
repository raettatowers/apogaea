// Lots of this from https://github.com/ThingPulse/esp32-icon64-a2dp
#include <atomic>
#include <arduinoFFT.h>
#include <BluetoothA2DPSink.h>
#include <cstdint>
#include <FastLED.h>

#include "animations.hpp"

// FFT Settings
const int BAND_COUNT = SPOKE_COUNT;
const int SAMPLE_COUNT = 512;
const int SAMPLING_FREQUENCY = 44100;
const float amplitude = 200.0;

// TODO: Do these need to be std::atomic?
extern CRGB leds[LED_COUNT];
extern std::atomic_bool renderSpectrumAnalyzer;
extern BluetoothA2DPSink bluetoothSink;
extern QueueHandle_t fftQueue;

static int32_t peak[BAND_COUNT] = {0};
static fft_t vReal[SAMPLE_COUNT];
static fft_t vImag[SAMPLE_COUNT];

// TODO: The 4th parameter is samplingFrequency, but I don't know if this is right
static arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLE_COUNT, SAMPLING_FREQUENCY);


void createBands(int i, int dsize) {
  uint8_t band = 0;
  // TODO: This should go up to 9 bands
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
  int dmax = amplitude;
  if (dsize > dmax)
    dsize = dmax;
  if (dsize > peak[band]) {
    peak[band] = dsize;
  }
}


void renderFFT(void*) {
  if (!renderSpectrumAnalyzer) {
    return;
  }
  if (!bluetoothSink.is_connected()) {
    for (byte spoke = 0; spoke < SPOKE_COUNT; ++spoke) {
      setLedHue(1, spoke, 0); // Just set ring to red
    }
    return;
  }

  int item = 0;
  for (;;) {
    if (uxQueueMessagesWaiting(fftQueue) > 0) {

      FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
      FFT.Compute(FFT_FORWARD);
      FFT.ComplexToMagnitude(vReal, vImag, SAMPLE_COUNT);

      for (uint8_t band = 0; band < BAND_COUNT; band++) {
        peak[band] = 0;
      }

      // Don't use sample 0 and only first NUM_SAMPLES/2 are usable. Each array eleement represents a frequency and its value the amplitude.
      for (int i = 2; i < (SAMPLE_COUNT / 2); i++) {
        if (vReal[i] > 2000) { // Add a crude noise filter, 10 x amplitude or more
          createBands(i, (int)vReal[i] / amplitude);
        }
      }

      // Release handle
      xQueueReceive(fftQueue, &item, 0);

      uint8_t intensity;

      for (byte band = 0; band < BAND_COUNT; band++) {
        intensity = map(peak[band], 1, amplitude, 0, BAND_COUNT);

        for (byte ring = 0; ring < RING_COUNT; ring++) {
          const int red = (ring >= intensity) ?  0 : ring;
          setLed(ring, band, red, 255, 255);
        }
      }

      /*
      if ((millis() - lastVisualizationUpdate) > 1000) {
        log_e("Fps: %f", visualizationCounter / ((millis() - lastVisualizationUpdate) / 1000.0));
        visualizationCounter = 0;
        lastVisualizationUpdate = millis();
        hueOffset += 5;
      }
      visualizationCounter++;
      */
    }
  }
}


void bluetoothDataCallback(const uint8_t *data, uint32_t length) {
  int item = 0;
  // Only prepare new samples if the queue is empty
  if (uxQueueMessagesWaiting(fftQueue) == 0) {
    // log_e("Queue is empty, adding new item");
    int byteOffset = 0;
    for (int i = 0; i < SAMPLE_COUNT; i++) {
      const int16_t sample_l_int = (int16_t)(((*(data + byteOffset + 1) << 8) | *(data + byteOffset)));
      const int16_t sample_r_int = (int16_t)(((*(data + byteOffset + 3) << 8) | *(data + byteOffset + 2)));
      vReal[i] = (sample_l_int + sample_r_int) / 2.0f;
      vImag[i] = 0;
      byteOffset = byteOffset + 4;
    }

    // Tell the task in core 1 that the processing can start
    xQueueSend(fftQueue, &item, portMAX_DELAY);
  }
}
