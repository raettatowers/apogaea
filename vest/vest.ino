#include <FastLED.h>
#include <math.h>
#include <SPI.h>
// RemoteXY select connection mode and include library
#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>
#include <RemoteXY.h>

#define REMOTEXY_BLUETOOTH_NAME "naivest"

#include "animations.hpp"
#include "constants.hpp"
#include "movies.hpp"

// RemoteXY configurate
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // 404 bytes
{ 255, 7, 0, 11, 0, 141, 1, 16, 24, 5, 2, 0, 4, 13, 22, 11, 1, 2, 26, 31,
  31, 99, 121, 99, 108, 101, 0, 115, 116, 97, 116, 105, 99, 0, 3, 10, 29, 13, 4, 40,
  1, 2, 26, 131, 1, 1, 1, 30, 7, 1, 2, 31, 77, 97, 105, 110, 0, 131, 0, 32,
  1, 30, 7, 2, 2, 31, 83, 68, 0, 129, 0, 35, 14, 12, 3, 1, 17, 80, 108, 97,
  115, 109, 97, 51, 0, 129, 0, 35, 22, 20, 3, 1, 17, 66, 105, 100, 111, 117, 108, 108,
  101, 32, 78, 101, 111, 110, 0, 129, 0, 35, 30, 24, 3, 1, 17, 72, 111, 114, 105, 122,
  111, 110, 116, 97, 108, 32, 83, 110, 97, 107, 101, 0, 129, 0, 35, 34, 20, 3, 1, 17,
  86, 101, 114, 116, 105, 99, 97, 108, 32, 83, 110, 97, 107, 101, 0, 129, 0, 35, 38, 20,
  3, 1, 17, 83, 112, 105, 114, 97, 108, 0, 129, 0, 35, 42, 20, 3, 1, 17, 83, 104,
  105, 110, 101, 0, 129, 0, 35, 46, 20, 3, 1, 17, 83, 112, 111, 116, 108, 105, 103, 104,
  116, 115, 0, 129, 0, 35, 50, 14, 3, 1, 17, 83, 112, 101, 99, 116, 114, 117, 109, 0,
  67, 5, 3, 21, 57, 5, 2, 2, 26, 11, 1, 0, 27, 29, 9, 9, 2, 164, 31, 0,
  1, 0, 16, 29, 9, 9, 2, 164, 31, 0, 1, 0, 38, 29, 9, 9, 2, 164, 31, 0,
  129, 0, 17, 40, 6, 3, 2, 17, 80, 114, 101, 118, 0, 129, 0, 39, 40, 6, 3, 2,
  17, 78, 101, 120, 116, 0, 129, 0, 28, 40, 6, 3, 2, 17, 80, 108, 97, 121, 0, 4,
  128, 3, 89, 18, 5, 0, 2, 26, 129, 0, 4, 84, 15, 3, 1, 17, 77, 97, 120, 32,
  112, 111, 119, 101, 114, 0, 129, 0, 35, 18, 21, 3, 1, 17, 66, 105, 100, 111, 117, 108,
  108, 101, 32, 80, 97, 115, 116, 101, 108, 0, 129, 0, 35, 26, 26, 3, 1, 17, 66, 105,
  100, 111, 117, 108, 108, 101, 32, 67, 104, 97, 110, 103, 105, 110, 103, 0, 4, 128, 24, 89,
  18, 5, 0, 2, 26, 129, 0, 24, 84, 15, 3, 1, 17, 66, 114, 105, 103, 104, 116, 110,
  101, 115, 115, 0
};

// this structure defines all the variables and events of your control interface
struct {

  // input variables
  uint8_t switch_cycle; // =1 if switch ON and =0 if OFF
  uint8_t animation; // =0 if select position A, =1 if position B, =2 if position C, ...
  uint8_t button_play; // =1 if button pressed, else =0
  uint8_t button_previous; // =1 if button pressed, else =0
  uint8_t button_next; // =1 if button pressed, else =0
  int8_t slider_max_power; // =0..100 slider position
  int8_t slider_brightness; // =0..100 slider position

  // output variables
  char text_sd[11];  // string UTF8 end zero

  // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;
#pragma pack(pop)

CRGB linearLeds[LED_COUNT];
CRGB* leds[] = {
  &linearLeds[LINEAR_LED_INDEXES[0]],
  &linearLeds[LINEAR_LED_INDEXES[1]],
  &linearLeds[LINEAR_LED_INDEXES[2]],
  &linearLeds[LINEAR_LED_INDEXES[3]],
  &linearLeds[LINEAR_LED_INDEXES[4]],
};


void setup() {
  Serial.begin(115200);
  Serial.println("setup");

  SPI.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(SD_PIN, OUTPUT);

  RemoteXY_Init();
  RemoteXY.slider_max_power = 50;
  RemoteXY.slider_brightness = 25;

  constexpr int pins[] = {17, 21, 4, 0, 15};
  for (const auto pin : pins) {
    pinMode(pin, OUTPUT);
  }
  FastLED.addLeds<WS2812, pins[0], GRB>(leds[0], STRAND_TO_LED_COUNT[0]);
  FastLED.addLeds<WS2812, pins[1], GRB>(leds[1], STRAND_TO_LED_COUNT[1]);
  FastLED.addLeds<WS2812, pins[2], GRB>(leds[2], STRAND_TO_LED_COUNT[2]);
  FastLED.addLeds<WS2812, pins[3], GRB>(leds[3], STRAND_TO_LED_COUNT[3]);
  FastLED.addLeds<WS2812, pins[4], GRB>(leds[4], STRAND_TO_LED_COUNT[4]);
  // Brightness and max power will be set below
  FastLED.clear();
  FastLED.show();

  Serial.println("exiting setup");
}

static uint8_t hue = 0;
static bool playingMovie = false;

static int soundFunction() {
  return 0;
}

static ColorGenerator hueGenerator;
static RedGreenGenerator redGreenGenerator;
static PastelGenerator pastelGenerator;
static NeonGenerator neonGenerator;
static ChangingGenerator changingGenerator;
static ChristmasGenerator christmasGenerator;

static Snake snake(10);
static HorizontalSnake horizontalSnake;
static Count count;
static CountXY countXY;
static Shine shine;
static Blobs blobs(4);
/*
static PlasmaBidoulleFast bidoulleChristmas(christmasGenerator);
static PlasmaBidoulleFast bidoulleNeon(neonGenerator);
static PlasmaBidoulleFast bidoulleRedGreen(redGreenGenerator);
static PlasmaBidoulleFast bidoullePastel(pastelGenerator);
static PlasmaBidoulleFast bidoulleChanging(changingGenerator);
*/
static Plasma3 plasma3(hueGenerator);
static BasicSpiral spiral(hueGenerator);
static MoviePlayer moviePlayer;

static SpectrumAnalyzer1 spectrumAnalyzer1(soundFunction);

//static constexpr Animation* animations[] = { &plasma3, &horizontalSnake, &snake, &spiral, &shine, &blobs };
//static constexpr Animation* animations[] = { &plasma3, &bidoullePastel, &bidoulleNeon, &bidoulleChanging, &horizontalSnake, &snake, &spiral, &shine, &blobs, &spectrumAnalyzer1 };
static constexpr Animation* const* animations[] = { &snake };

void loop() {
  const int hueDuration_ms = 50;

  unsigned long hueStart_ms = millis();
  while (true) {
    if (millis() > hueStart_ms + hueDuration_ms) {
      hueStart_ms = millis();
      ++hue;
    }

    const int delay_ms = playingMovie ? moviePlayer.animate(0) : animations[RemoteXY.animation]->animate(hue);
    FastLED.show();
    delayAndHandleRemoteXy(delay_ms);
  }
}

static void delayAndHandleRemoteXy(const int delay_ms) {
  static uint8_t previousSwitchCycle = RemoteXY.switch_cycle;

  // TODO: Make this a slider too?
  const int animationDuration_ms = 10000;
  const decltype(millis()) start = millis();

  static decltype(millis()) animationStart_ms = millis();

  do {
    RemoteXY_Handler();
  } while (millis() < start + delay_ms);

  if (previousSwitchCycle != RemoteXY.switch_cycle) {
    animationStart_ms = millis();
    playingMovie = false;
  }
  previousSwitchCycle = RemoteXY.switch_cycle;

  if (RemoteXY.button_play) {
    playingMovie = true;
  }
  if (RemoteXY.button_next) {
    if (moviePlayer) {
      moviePlayer.next(RemoteXY.text_sd, COUNT_OF(RemoteXY.text_sd));
    } else {
      const char message[] = "FAIL";
      static_assert(COUNT_OF(message) < COUNT_OF(RemoteXY.text_sd));
      strcpy(RemoteXY.text_sd, message);
    }
  } else if (RemoteXY.button_previous) {
    if (moviePlayer) {
      moviePlayer.previous(RemoteXY.text_sd, COUNT_OF(RemoteXY.text_sd));
    } else {
      const char message[] = "FAIL";
      static_assert(COUNT_OF(message) < COUNT_OF(RemoteXY.text_sd));
      strcpy(RemoteXY.text_sd, message);
    }
  }

  if (RemoteXY.switch_cycle && millis() > animationStart_ms + animationDuration_ms) {
    animationStart_ms = millis();
    ++RemoteXY.animation;
    if (RemoteXY.animation > COUNT_OF(animations)) {
      RemoteXY.animation = 0;
    }
  }

  // Set max milliamps between 1000 and 2000
  FastLED.setMaxPowerInVoltsAndMilliamps(5, RemoteXY.slider_max_power * 10 + 1000);

  const int brightness = max(RemoteXY.slider_brightness * 255 / 100, 5);
  FastLED.setBrightness(brightness);
}
