#ifndef ANIMATIONS_HPP
#define ANIMATIONS_HPP

#include <cstdint>

#define COUNT_OF(x) ((sizeof(x) / sizeof(0 [x])))

const int LED_COUNT = 50;
// The outer ring has 18, but the inner ones only have 9
const int SPOKE_COUNT = 18;
const int RING_COUNT = 5;
static_assert(SPOKE_COUNT / 2 * RING_COUNT + RING_COUNT == LED_COUNT);

void setLed(int ring, int spoke, std::uint8_t red, std::uint8_t green, std::uint8_t blue);
void setLedHue(int ring, int spoke, std::uint8_t hue);
void hsvToRgb(std::uint8_t hue, std::uint8_t saturation, std::uint8_t value, std::uint8_t *red,
              std::uint8_t *green, std::uint8_t *blue);

int lightAll();
int spinSingle();
int fastOutwardHue();
int fastInwardHue();
int spiral();
int outwardRipple();
int outwardRippleHue();
int singleSpiral();
int blurredSpiral();
int blurredSpiralHues();
int orbit();
int triadOrbits();
int pendulum();
int comets();
int cometsShort();
int fadingRainbowRings();
int outerHue();
int outerRipple();

#endif
