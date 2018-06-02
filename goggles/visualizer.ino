/* 5x6 LED Visualized, Created by u/HomelessHamSandwich, 2017
 
   Includes code adapted from these tutorials:
   FastLED Tutorial: https://randomnerdtutorials.com/guide-for-ws2812b-addressable-rgb-led-strip-with-arduino/
   Arduino FFT Tutorial: https://www.norwegiancreations.com/2017/08/what-is-fft-and-how-can-you-implement-it-on-an-arduino/
 
   Code is not built for adaptability, but it shouldn't be too hard to make it work for other LED matrices!
*/
 
#include <FastLED.h>
 
#include <arduinoFFT.h>
 
#define SAMPLES 64              //Must be a power of 2
#define SAMPLING_FREQUENCY 9600 //Hz, must be less than 10000 due to ADC
 
#define LED_PIN     5
#define NUM_LEDS    30
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
 
#define UPDATES_PER_SECOND 50
 
arduinoFFT FFT = arduinoFFT();
 
unsigned int sampling_period_us;
unsigned long microseconds;
 
double vReal[SAMPLES];
double vImag[SAMPLES];
 
/* Initializing values for visualizer */
int prevFreq1 = 50;
int prevFreq2 = 50;
int prevFreq3 = 50;
int prevFreq4 = 50;
int prevFreq5 = 50;
int prevFreq6 = 50;
 
CRGBPalette16 currentPalette;
TBlendType    currentBlending;
 
void setup() {
 
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
 
  delay(3000); // Safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
 
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
 
}
 
void loop() {
 
  /*Assigning leds to columns*/
  int colA[] = {24, 18, 12, 6, 0};
  int colB[] = {25, 19, 13, 7, 1};
  int colC[] = {26, 20, 14, 8, 2};
  int colD[] = {27, 21, 15, 9, 3};
  int colE[] = {28, 22, 16, 10, 4};
  int colF[] = {29, 23, 17, 11, 5};
 
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1;
 
  /*SAMPLING*/
 
  for (int i = 0; i < SAMPLES; i++)
  {
    microseconds = micros();    //Overflows after around 70 minutes!
    analogReference(EXTERNAL);
    vReal[i] = analogRead(0);
    vImag[i] = 0;
 
    while (micros() < (microseconds + sampling_period_us)) {
    }
  }
 
  /*FFT*/
 
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_RECTANGLE, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
 
  /* Visualizer
 
     Averages previous and current sample values to create smoother values and control noise.
 
     vReal is an array of values picked up by the microphone. The frequency of each value corresponds to ( (SAMPLING_FREQUENCY / (SAMPLES/2)) * (Index of vReal + 1) ).
     For example, the frequency of vReal[4] for SAMPLING_FREQUENCY = 9600, SAMPLES = 64 would be 1,500 Hz.
 
     Bias values allow for easy adjustment of each column, since lower frequencies are generally picked up at higher levels.
 
  */
 
  prevFreq1 = SpecGraph((((vReal[5]) + prevFreq1) / 2), colA, startIndex, 1.3);
  prevFreq2 = SpecGraph((((vReal[7]) + prevFreq2) / 2), colB, startIndex, 1.6);
  prevFreq3 = SpecGraph((((vReal[9]) + prevFreq3) / 2), colC, startIndex, 1.9);
  prevFreq4 = SpecGraph((((vReal[11]) + prevFreq4) / 2), colD, startIndex, 2.0);
  prevFreq5 = SpecGraph((((vReal[13]) + prevFreq5) / 2), colE, startIndex, 2.1);
  prevFreq6 = SpecGraph((((vReal[15]) + prevFreq6) / 2), colF, startIndex, 2.1);
 
  FastLED.show();
  FastLED.delay(1000 / UPDATES_PER_SECOND);
  delay(20);
}
 
 
/* Lights up the leds in a given column based on the levels detected for a given frequency.
   MUCH easier to tweak BIAS when called than to edit cutoff levels. */
 
int SpecGraph( uint32_t FREQ, int COLUMN[], uint8_t colorIndex, double BIAS ) {
  if ((FREQ * BIAS) > 550.0) // Cutoff level
  {
    for ( int i = 0; i < 5; i++) {
      leds[COLUMN[i]] = ColorFromPalette( currentPalette, colorIndex, 255, currentBlending);
      colorIndex += 3;
    }
  }
  if ((FREQ * BIAS) > 500.0)
  {
    for ( int i = 0; i < 4; i++) {
      leds[COLUMN[i]] = ColorFromPalette( currentPalette, colorIndex, 255, currentBlending);
      colorIndex += 3;
    }
  }
  if ((FREQ * BIAS) > 450.0)
  {
    for ( int i = 0; i < 3; i++) {
      leds[COLUMN[i]] = ColorFromPalette( currentPalette, colorIndex, 255, currentBlending);
      colorIndex += 3;
    }
  }
  if ((FREQ * BIAS) > 350.0)
  {
    for ( int i = 0; i < 2; i++) {
      leds[COLUMN[i]] = ColorFromPalette( currentPalette, colorIndex, 255, currentBlending);
      colorIndex += 3;
    }
  }
  if ((FREQ * BIAS) > 300.0)
  {
    for ( int i = 0; i < 1; i++) {
      leds[COLUMN[i]] = ColorFromPalette( currentPalette, colorIndex, 255, currentBlending);
      colorIndex += 3;
    }
  }
  if ((FREQ * BIAS) <= 300.0)
  {
    for ( int i = 0; i < 5; i++) {
      leds[COLUMN[i]] = CRGB::Black; // Clears column if under threshold
    }
  }
  return (FREQ); // Returns the level given for FREQ. This lets you hold on to the given value, which can be saved and then used to average current and previous values.
}
