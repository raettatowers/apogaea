// FastLED doesn't work with nrf52 and APA102 LED strips yet
#include <Adafruit_DotStar.h>

#include "animations.hpp"
#include "constants.hpp"

Adafruit_DotStar leds(LED_COUNT, DATA_PIN, CLOCK_PIN, DOTSTAR_BRG);

SingleColor singleColor(0x00FF00);
RainbowColors rainbowColors(3000, 300);

InchWormAnimation inchWorm(leds, LED_COUNT, 6, 0);
SpectrumAnalyzer analyzer(leds, LED_COUNT);

void setup() {
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  //pinMode(NEOPIXEL_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);
  const int speakerShutDownPin = 11;
  pinMode(speakerShutDownPin, OUTPUT);
  digitalWrite(speakerShutDownPin, LOW);  // Save power

  leds.begin();

  Serial.begin(9600);
}

void loop() {
  leds.clear();

  //inchWorm.animate(rainbowColors);
  analyzer.animate(rainbowColors);

  rainbowColors.nextFrame();
  leds.show();
  delay(100);
}

bool buttonPressed() {
  static int previousButtonState;

  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == HIGH && buttonState != previousButtonState) {
    previousButtonState = buttonState;
    delay(10); // Lazy debounce
    return true;
  }
  previousButtonState = buttonState;
  return false;
}
