LED vest
========

Finchronicity's vest
--------------------

- 470 WS2811 LEDs, 30/m, 15m
- 12mm clear heat shrink

My vest v2
----------

- 5 seed strands
- Wires:
  - White
    - IO17/GPIO17
    - central upper stage left
    - LEDs 0-4 are unused
    - LED 52 is the last used
  - Blue
    - IO21/GPIO21
    - stage right
    - LEDs 0-1 are unused
    - LED 101 is the last used
  - Red
    - IO4/GPIO4
    - central upper stage right
    - LEDs 0-3 are unused
    - LED 64 is the last used
  - Green
    - IO0/GPIO0
    - central lower
    - LEDs 0-4 are unused
    - LED 53 is the last used
  - Black
    - TD0/GPIO15
    - stage left
    - LED 0 is unused
    - LED 63 is the last used
- Total size is 30 width x 15 height
- Mappings, considering that 0, 0 is lower stage left?
  - Stage right goes from x:[0-10] and y:[4-14]
  - Central goes from x:[11-23] and y:[0-14]
  - Stage left goes from x:[24-31] and y:[4-14]
  - Doing numbering in x,y coordinates with runs specified as start..end inclusive
  - See offsets.py

My vest v1
----------

- 95cm wide, center is 30cm wide, 65cm tall 
- 300 WS2811 LEDs, 30/m, 5m, so each LED is 3.33cm apart
- Strips I want to do:
  - 50 * 5, 35 * 2, 40 * 4 = 500, 11 strips = 1 per 8.6cm
  - 50 * 4, 40 * 2, 35 * 4, 30 * 2 = 480, 10 strips = 1 per 9.5cm
  - 50 * 6, 40 * 2, 35 * 2, 25 * 2 = 500, 12 strips = 1 per 7.9cm
15 middle, 6 12 18 24 30, 
