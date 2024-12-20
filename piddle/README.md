Piddle
======

Knockoff version of Sound Puddle. Smaller, fewer strips, running on a protoboard ESP32.

Wiring
------

- LED pins: see source
- Microphone
  - SCK: 19
  - WS: 22
  - SD: 21
  - L/R: ground
  - VDD: 3.3V
  - GND: ground

Hardware
--------

- ESP32 WROOM 32
- I2S microphone
- 10 WS2812B LED strips

PCB notes
---------
Added ground plane - select F. CU (front copper) and then add a filled zone
Want ground plane everywhere
Can do vias
JLC PCB is good, they're fast, but less quality then PCBWay
