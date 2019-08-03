#include <Adafruit_NeoPixel.h>
#include <Adafruit_DotStar.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
#include <avr/power.h>
#endif
#include <fix_fft.h>
#include <arduinoFFT.h>

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

// On Uno, if you're using Serial, this needs to be > 1. On Trinket it should be 0.
const int NEOPIXELS_PIN = 0;
const int BUTTON_PIN = 3;
const int ONBOARD_LED_PIN = 13;
const int MICROPHONE_ANALOG_PIN = A1;
// Sample count must be a power of 2. I chose 128 because only half of the values
// from the FFT correspond to frequencies, and the first 2 are sample averages, so
// I needed more than 2 * PIXEL_RING_COUNT * 2 + 2, which gives me 128.
const int SAMPLE_COUNT = 128;
const int PIXEL_RING_COUNT = 16;
const int MODE_TIME_MS = 8000;

Adafruit_DotStar internalPixel = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_RING_COUNT * 2, NEOPIXELS_PIN);

uint16_t hue = 0;  // Start red
uint32_t color = pixels.ColorHSV(hue);


void setup() {
#ifdef __AVR_ATtiny85__  // Trinket, Gemma, etc.
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin();
  pixels.setBrightness(20);
  clearLeds();

  internalPixel.begin();
  // Just shut it off
  internalPixel.setPixelColor(0, 0);
  internalPixel.show();

  analogReference(AR_DEFAULT);
  pinMode(MICROPHONE_ANALOG_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(NEOPIXELS_PIN, OUTPUT);
  pinMode(ONBOARD_LED_PIN, OUTPUT);
}


void spectrumAnalyzer() {
  // Most songs have notes in the lower end, so from experimental
  // observation, this seems like a good choice
  const uint32_t SAMPLING_FREQUENCY_HZ = 5000;
  const uint32_t samplingPeriod_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY_HZ));
  static arduinoFFT FFT = arduinoFFT();
  static uint8_t previousValues[2 * PIXEL_RING_COUNT] = {0};
  // Change the base hue of low intensity sounds so we can get more colors
  static uint32_t baseHue = 0;

  double vReal[SAMPLE_COUNT];
  double vImaginary[SAMPLE_COUNT];
  double sampleAverages[2 * PIXEL_RING_COUNT];
  const int usableValues = COUNT_OF(vReal) / 2;
  const int stepSize = usableValues / COUNT_OF(sampleAverages);

  baseHue += 100;

  for (int i = 0; i < SAMPLE_COUNT; ++i) {
    // TODO: Do we need to worry about overflow?
    const uint32_t before = micros();

    vReal[i] = analogRead(MICROPHONE_ANALOG_PIN);
    vImaginary[i] = 0;

    const uint32_t target_us = before + samplingPeriod_us;
    const uint32_t now = micros();
    // I don't know if this is better than a busy wait loop or not
    delayMicroseconds(before + samplingPeriod_us - now);
  }

  FFT.Windowing(vReal, SAMPLE_COUNT, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImaginary, SAMPLE_COUNT, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImaginary, SAMPLE_COUNT);

  // The first 2 samples? are the average power of the signal, so skip them.
  // Samples < 20Hz are so low that humans can't hear them, so skip those too.
  const int SKIP = 2 + 5;
  uint8_t counter = SKIP;
  for (uint16_t i = 0; i < COUNT_OF(sampleAverages); ++i) {
    sampleAverages[i] = 0;
    // The FFT gives us equally spaced buckets (e.g. bucket 1 is 0-100 Hz, bucket 2 is 100-200 Hz)
    // but notes increase in frequency quadratically. In fact, each increase in octave is double
    // the frequency. As a kludge, just skip every other sample for the first half of the samples.
    const int increment = i < COUNT_OF(sampleAverages) / 2 ? 2 : 1;
    for (int j = 0; j < stepSize; j += increment, ++counter) {
      sampleAverages[i] += vReal[counter];
    }
    // This division should compile down to a right bit shift
    sampleAverages[i] /= stepSize;
    // To be accurate, we should multiply by the increment. However, I've found that percussion
    // tends to be lower, and it overwhelms the other stuff. Since we're biasing against low sounds
    // anyway, we just won't do that. I think the visualization looks better. Sorry bass line.
  }

  double max_ = -1;
  for (auto sa : sampleAverages) {
    max_ = max(max_, sa);
  }
  // Set a default max so that if it's quiet, we're not visualizing random noises
  max_ = max(max_, 400);
  const double MULTIPLIER = 1.0 / max_;
  // Clamp them all to 0.0 - 1.0
  for (auto& sa : sampleAverages) {
    sa *= MULTIPLIER;
  }

  const uint8_t MAX_BRIGHTNESS = 128;
  // This cutoff needs some tweaking based on pixels.getBrightness()
  // 10 works with brightness = 20
  const uint8_t CUTOFF = 10;

  for (uint8_t i = 0; i < COUNT_OF(sampleAverages); ++i) {
    // Implement a fade-off effect
    const int FADE_OFF = 20;
    const uint8_t value_ = max(previousValues[i] > FADE_OFF ? previousValues[i] - FADE_OFF : 0, sampleAverages[i] * 0xFF);
    previousValues[i] = value_;
    const uint32_t hue = static_cast<uint32_t>(value_) * 0xFF + baseHue;
    uint8_t brightness = min(MAX_BRIGHTNESS, value_);
    // The visualization looks like garbage when brightness is high because all of the
    // pixels turn on, and I want dim pixels off
    if (brightness < CUTOFF) {
      brightness = 0;
    }
    pixels.setPixelColor(i, pixels.ColorHSV(hue, 0xFF, brightness));
  }

  pixels.show();
}


void randomSparks() {
  const uint8_t led1 = random(PIXEL_RING_COUNT);
  pixels.setPixelColor(led1, color);
  const uint8_t led2 = random(PIXEL_RING_COUNT) + PIXEL_RING_COUNT;
  pixels.setPixelColor(led2, color);
  pixels.show();
  delay(20);
  pixels.setPixelColor(led1, 0);
  pixels.setPixelColor(led2, 0);
  pixels.show();
}


void fadingSparks() {
  // Like random sparks, but they fade in and out
  static bool increasing[2 * PIXEL_RING_COUNT] = {
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
  };
  static int8_t brightnessIndexes[2 * PIXEL_RING_COUNT] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
  };
  // Let's keep them the same color that they started with
  static uint16_t hues[2 * PIXEL_RING_COUNT] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
  };
  // Let's use our own sparkHue so that we can change the pixels more quickly
  static uint16_t sparkHue = 0;
  // Start with some zeroes so that we don't relight a pixel immediately
  const static uint8_t brightnesses[] = {0, 0, 0, 0, 0, 0, 4, 8, 16, 32, 48, 64, 64, 64};

  // So we pick a random LED to start making brighter
  const uint8_t led = random(2 * PIXEL_RING_COUNT);
  if (!increasing[led]) {
    increasing[led] = true;
    hues[led] = sparkHue;
    sparkHue += 1200;
  }

  for (uint8_t i = 0; i < COUNT_OF(increasing); ++i) {
    if (increasing[i]) {
      if (brightnessIndexes[i] < static_cast<int>(COUNT_OF(brightnesses))) {
        ++brightnessIndexes[i];
      } else {
        increasing[i] = false;
      }
      pixels.setPixelColor(i, pixels.ColorHSV(hues[i], 0xFF, brightnesses[brightnessIndexes[i]]));
    } else {
      if (brightnessIndexes[i] > 0) {
        --brightnessIndexes[i];
        pixels.setPixelColor(i, pixels.ColorHSV(hues[i], 0xFF, brightnesses[brightnessIndexes[i]]));
      }
    }
  }
  delay(50);
  pixels.show();
}


void binaryClock() {
  // Do a binary shift instead of integer division because of speed and code size.
  // It's a little less precise, but who cares.
  // Show 1/4 seconds instead of full seconds because it's more interesting. It
  // updates a lot faster and the other lens lights up faster.
  uint32_t now = millis() >> 8;
  showNumber(now, color);
  delay(100);
}


void spinnyWheels() {
  static uint8_t offset = 0;  // Position of spinny eyes
  for (uint8_t i = 0; i < PIXEL_RING_COUNT; ++i) {
    uint32_t c = 0;
    if (((offset + i) & 0b111) < 2) {
      c = color;  // 4 pixels on...
    }
    pixels.setPixelColor(i, c);  // First eye
    pixels.setPixelColor(PIXEL_RING_COUNT * 2 - 1 - i, c);  // Second eye (flipped)
  }
  pixels.show();
  ++offset;
  hue += 20;  // Make the rainbow colors rotate faster
  delay(50);
}


void showNumber(uint32_t number, const uint32_t color) {
  uint8_t counter = 0;
  while (number > 0) {
    if (number & 1) {
      pixels.setPixelColor(counter, color);
    } else {
      pixels.setPixelColor(counter, 0);
    }
    number >>= 1;
    counter += 2;
  }
  pixels.show();
}


void swirls() {
  static uint8_t head1 = 0;
  static uint8_t head2 = 3;
  static const uint8_t brightness[] = {255, 128, 64, 32, 16, 8, 4, 2, 1};

  // I could probably optimize this loop so that I'm not going through the whole ring
  for (uint8_t i = 0; i < PIXEL_RING_COUNT; ++i) {
    const int8_t difference1 = head1 - i;
    if (0 <= difference1 && difference1 < static_cast<int>(COUNT_OF(brightness))) {
      pixels.setPixelColor(i, pixels.ColorHSV(hue, 0xFF, brightness[difference1]));
    } else {
      const int8_t difference2 = PIXEL_RING_COUNT + difference1;
      if (0 <= difference2 && difference2 < static_cast<int>(COUNT_OF(brightness))) {
        pixels.setPixelColor(i, pixels.ColorHSV(hue, 0xFF, brightness[difference2]));
      }
    }
  }
  head1 = (head1 + 1) % PIXEL_RING_COUNT;

  // Make the second lens swirl the other direction
  for (uint8_t i = 0; i < PIXEL_RING_COUNT; ++i) {
    const int8_t difference1 = head2 - i;
    if (0 <= difference1 && difference1 < static_cast<int>(COUNT_OF(brightness))) {
      pixels.setPixelColor(PIXEL_RING_COUNT * 2 - 1 - i, pixels.ColorHSV(hue, 0xFF, brightness[difference1]));
    } else {
      const int8_t difference2 = PIXEL_RING_COUNT + difference1;
      if (0 <= difference2 && difference2 < static_cast<int>(COUNT_OF(brightness))) {
        pixels.setPixelColor(PIXEL_RING_COUNT * 2 - 1 - i, pixels.ColorHSV(hue, 0xFF, brightness[difference2]));
      }
    }
  }
  head2 = (head2 + 1) % PIXEL_RING_COUNT;

  pixels.show();
  delay(50);
}


void shimmer() {
  // Start above 0 so that each light should be on a bit
  static const uint8_t brightness[] = {4, 8, 16, 32, 64, 128};
  static uint8_t values[PIXEL_RING_COUNT * 2] = {
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
    COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2, COUNT_OF(brightness) / 2,
  };

  for (int i = 0; i < 4; ++i) {
    const uint8_t pixel = random(PIXEL_RING_COUNT * 2);
    if (random(2) == 1) {
      if (values[pixel] < COUNT_OF(brightness)) {
        ++values[pixel];
      } else {
        --values[pixel];
      }
    } else {
      if (values[pixel] > 0) {
        --values[pixel];
      } else {
        ++values[pixel];
      }
    }
  }

  for (int i = 0; i < PIXEL_RING_COUNT * 2; ++i) {
    pixels.setPixelColor(i, pixels.ColorHSV(hue, 0xFF, brightness[values[i]]));
  }
  pixels.show();
  delay(50);
}


void rainDrops() {
  static uint8_t brightnesses[2 * PIXEL_RING_COUNT] = {0};
  static int8_t directions[COUNT_OF(brightnesses)] = {0};
  static uint8_t delays[COUNT_OF(brightnesses)] = {0};
  static uint8_t maxBrightnesses[COUNT_OF(brightnesses)] = {0};

  // Each pixel operates independently without looking at its neighbors,
  // increasing up to some max brightness then turning around and decreasing to
  // 0 and then increasing again. Each time max is reached, the max is
  // decreased.  To implement a ripple effect, a delay is introduced for each
  // pixel the further it is from the initial drop before it starts running the
  // cycle.

  const int RIPPLE_DROP_OFF = 5;
  const int MAX_DROP_OFF = 15;
  const int INCREMENT = 20;
  const int MAX_BRIGHTNESS = 100;
  const int INITIAL_DELAY = 3;
  static_assert(MAX_BRIGHTNESS > COUNT_OF(brightnesses) / 2 * RIPPLE_DROP_OFF, "");

  bool allOff = true;
  for (auto b : brightnesses) {
    if (b > 0) {
      allOff = false;
      break;
    }
  }
  if (allOff) {
    for (auto &d : directions) {
      d = 1;
    }
    // Start next drop
    const uint8_t drop = random(COUNT_OF(brightnesses));
    delays[drop] = 0;
    maxBrightnesses[drop] = MAX_BRIGHTNESS;
    // Prepare the other drops
    for (uint8_t i = 1; i < COUNT_OF(brightnesses) / 2 + 1; ++i) {
      const int index1 = (COUNT_OF(brightnesses) * 2 + drop - i) % COUNT_OF(brightnesses);
      const int index2 = (COUNT_OF(brightnesses) + drop + i) % COUNT_OF(brightnesses);
      delays[index1] = i * INITIAL_DELAY;
      delays[index2] = i * INITIAL_DELAY;
      maxBrightnesses[index1] = MAX_BRIGHTNESS - i * RIPPLE_DROP_OFF;
      maxBrightnesses[index2] = MAX_BRIGHTNESS - i * RIPPLE_DROP_OFF;
    }
  }

  // Update the drops
  for (uint8_t i = 0; i < COUNT_OF(brightnesses); ++i) {
    if (delays[i] == 0) {
      // Increase toward the max
      if (directions[i] > 0) {
        if (brightnesses[i] + INCREMENT <= maxBrightnesses[i]) {
          brightnesses[i] += INCREMENT;
        } else {
          // Decrease max brightness if we can
          if (maxBrightnesses[i] - MAX_DROP_OFF > 0) {
            brightnesses[i] = maxBrightnesses[i];
            maxBrightnesses[i] -= MAX_DROP_OFF;
            directions[i] = -1;
          } else {
            // All done
            maxBrightnesses[i] = 0;
            brightnesses[i] = 0;
            directions[i] = 0;
          }
        }
      // Decrease toward zero
      } else if (directions[i] < 0) {
        if (brightnesses[i] < INCREMENT) {
          // Turn around
          directions[i] = 1;
          brightnesses[i] = 0;
          // Also add a delay to get a better ripple effect
          delays[i] = 3;
        } else {
          brightnesses[i] -= INCREMENT;
        }
      }
    } else {
      --delays[i];
    }

    pixels.setPixelColor(i, pixels.ColorHSV(hue, 0xFF, brightnesses[i]));
  }

  pixels.show();
  delay(100);
}


void clearLeds() {
  for (int i = 0; i < PIXEL_RING_COUNT * 2; ++i) {
    pixels.setPixelColor(i, 0);
  }
}


void configureBrightness(const bool buttonPressed) {
  static uint8_t targetBrightness = 1;

  if (buttonPressed) {
    pixels.setBrightness((256 / (PIXEL_RING_COUNT * 2)) * targetBrightness - 1);
  }

  pixels.setBrightness(10);
  for (int i = 0; i < PIXEL_RING_COUNT * 2; ++i) {
    if (i <= targetBrightness) {
      pixels.setPixelColor(i, color);
    }
  }
  pixels.show();
  delay(250);

  ++targetBrightness;
  if (targetBrightness == PIXEL_RING_COUNT * 2) {
    targetBrightness = 0;
  }
}


static uint8_t mode = 0;  // Current animation effect
static uint8_t animationsIndex = 0;
typedef void (*animationFunction_t)();
// Each animationFunction_t[] should end in nullptr
const animationFunction_t ALL_ANIMATIONS[] = {spinnyWheels, binaryClock, fadingSparks, shimmer, swirls, spectrumAnalyzer, nullptr};
const animationFunction_t ONLY_ANIMATIONS[] = {spinnyWheels, binaryClock, fadingSparks, shimmer, swirls, nullptr};
const animationFunction_t ONLY_SPECTRUM_ANALYZER[] = {spectrumAnalyzer, nullptr};
//const animationFunction_t* ANIMATIONS_LIST[] = {ALL_ANIMATIONS, ONLY_ANIMATIONS, ONLY_SPECTRUM_ANALYZER};
// Use this for testing a single animation
const animationFunction_t ONLY_RAIN_DROPS[] = {rainDrops, nullptr};
const animationFunction_t* ANIMATIONS_LIST[] = {ONLY_RAIN_DROPS};


typedef void (*configurationFunction_t)(bool);
void loop() {
  static uint32_t modeStartTime_ms = millis();
  static uint32_t buttonPressTime = 0;

  bool buttonPressed = false;

  if (digitalRead(BUTTON_PIN) == HIGH) {
    // Debounce
    if (millis() - buttonPressTime > 100) {
      buttonPressTime = millis();
      buttonPressed = true;
    }
  }

  if (buttonPressed) {
    // TODO: More complex configuration
    ++animationsIndex;
    if (animationsIndex == COUNT_OF(ANIMATIONS_LIST)) {
      animationsIndex = 0;
    }
    mode = 0;
  }

  // Do a regular animation
  const uint32_t startAnimation_ms = millis();
  const animationFunction_t* animations = ANIMATIONS_LIST[animationsIndex];
  animations[mode]();

  // Update the color
  const uint32_t now_ms = millis();
  // We want to complete a full hue color cycle about every X seconds
  const int hueCycle_ms = 20000;
  const int hueCycleLimit = 65535;
  hue += (now_ms - startAnimation_ms) * hueCycleLimit / hueCycle_ms;
  color = pixels.ColorHSV(hue);

  if ((now_ms - modeStartTime_ms) > MODE_TIME_MS) {
    ++mode;
    if (animations[mode] == nullptr) {
      mode = 0;
    }
    clearLeds();
    modeStartTime_ms = now_ms;
  }
}
