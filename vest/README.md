LED vest
========

A glowing furry vest. Copied from an idea from Finchronicity.

- [Hackaday LED vest](https://hackaday.io/project/395-led-vest/log/570-power)
- [PJRC forums LED vest]([https://forum.pjrc.com/threads/25939-Led-vest)
- [Finchronicity's source code](https://github.com/finominal/LedVestPlasmaGenerator)

Parts
-----

- White plush faux fur, Joann Fabrics, 1.5 yards: $48
- Liner fabric, Joann Fabrics, 2 yards: $15
- LED strip, 5m 30 LED/m WS2812B 5V white PCB IP65, BTF-Lighting: $22
- Arduino-compatible controller board. A couple good choices:
  - PJRC Teensy 4.1, comes with a MicroSD reader: $27
  - Adafruit Circuit 
  - Adafruit Trinket M0: $9
  - Uno probably won't work, probably doesn't have enough memory
- Wire
- Soldering iron and solder
- Hot glue gun
- Capacitor, 500-1000 uF at 6.3V or higher
- Resistor, 300-500 ohm
- Sewing machine, thread, and hand needle

Sewing
------

I used this [pattern-free furry vest
tutorial](https://www.youtube.com/watch?v=pE6P3lnAmCU). I had originally bought
a men's vest pattern, but those are meant to be form-fitting and definitely
overkill. The vest is supposed to be loose and fit over other clothing, so
there's a lot of play when it comes to sewing and cutting. If you find it
useful, [consider supporting her on
Ko-Fi](https://ko-fi.com/thoughtful_creativity)

I bought white fur and
some space-themed liner from Joann Fabrics. I found that white fur works best;
it diffuses the LEDs well and doesn't block much color. I used the
longest-haired fur they had - shorter fur doesn't diffuse as well and you can
see the individual LEDs, which detracts from the effect. I sewed channels
connecting the liner and the fur that I threaded the LED strips through. One
difference you need to follow from the guide is that you don't want to sew the
tops or bottoms of the vest. That way, you can insert and solder the LED
strips.

The LED strips I used have 30 LEDs / meter, or 3.33cm apart, so the channels
are the same. That way the LEDs appear evenly spaced and not like there are a
bunch of strips.

I sewed two pockets in the vest on the inside: one to hold the battery pack,
and one to hold the controller. Normally when you sew a pocket, you take a
single piece of fabric and attach the sides and bottom by sewing. However,
attaching the bottom would block some LED channels. Instead I made a pocket
out of two pieces of cloth then attached it by only sewing the sides over an
existing channel.

Lighting
--------

### Finchronicity's vest

- 470 WS2811 LEDs, 30/m, 15m
- 12mm clear heat shrink

### My vest

My original plan was this:

- 95cm wide, center is 30cm wide, 65cm tall 
- 300 WS2812B LEDs, 30/m, 5m, so each LED is 3.33cm apart
- Strips I want to do:
  - 50 * 5, 35 * 2, 40 * 4 = 500, 11 strips = 1 per 8.6cm
  - 50 * 4, 40 * 2, 35 * 4, 30 * 2 = 480, 10 strips = 1 per 9.5cm
  - 50 * 6, 40 * 2, 35 * 2, 25 * 2 = 500, 12 strips = 1 per 7.9cm
- I wound up with 264 LEDs

Power
-----

For simplicity, my entire system was 5V. That way I could power it off of a USB
power pack.

At maximum brightness and displaying white, WS2812B LEDs each use about 250 mW.
If you're only displaying 1 color channel, the power will be reduced to 1/3.
You can also reduce the brightness, which honestly looks fine.

The FastLED Arduino library can limit the total amount of power that it uses at
any given time by dimming all the LEDs, so you can budget how big of a battery
you need.

I have 264 LEDs, so the maximum power usage would be 66W. To power that, I'd
probably need to carry something large like a motorcycle battery and step the
voltage down to 5V. The largest USB power pack I could find can put out 15W, so
I'm just running it at a lower brightness. A lot of USB battery packs list the
capacity in amp-hours instead of watt-hours, and even though the output is 5V,
the internal battery is 3.7, so you need to multiply by 3.7 to get the
watt-hours. That big pack is 72 watt-hours, so at maximum power draw, it could
run a little under 5 hours.

Wiring
------

I kind of winged it once I measured the vest, but made certain to keep the
number of LEDs symmetrical. I ended up with 264 LEDs.

The LED data lines need to be hooked up serially. That means I had one strip
going up, then its pair going down. As such, I connected two strips at a time,
and then inserted them into the vest. I cut each strip to length on its
solder pads and then soldered the 3 pads together using some wire. Before
soldering anything, I recommend labeling the data, voltage, and ground pads;
the data direction; and the number of LEDs on the back of the strip using a
permanent marker. Trying to count LEDs when they're pulled through a channel is
a pain. After soldering together each pair, I connected them to an Arduino with
a small test program just to make sure that the whole strip, including both
halves, was working.

Once I verified that the pair of strips was good, I threaded them through the
channels. First I hot glued the solder joints to try to reinforce and insulate
them. Then I threaded the whole strip through shrink wrap to try to protect
them. Some LED strips come with plastic protection which would have been
easier. You can't push the strip through the channel, so I tied a string to
safety pin and used a magnet to drag it through a pair of channels from the
bottom. Then I hooked each strip to a safety pin and pulled it back through,
making sure the LEDs were facing outward. Some strips twisted and didn't want
to lay flat, so I had to clip the shrink wrap around the connecting wire at
the top.

The voltage and ground lines need to be hooked up in parallel. The power can't
be serial because the voltage drops over length. I added wires to the bottom of
the vest for voltage and ground and attached the wires to the vest by sewing a
few loops of thread. I needed to strip the wire in the middle to expose them
for each line. I did that by twisting a wirecutter around the wire in two
spots, then using a razor to scrape away the insulation. To solder, I wrapped a
couple loops around the power line, soldered it, and then covered it in glue to
insulate and strengthen the connection.

To connect the power lines to the battery pack, I stripped a USB A cable to
expose the voltage and ground wires and soldered them to the parallel wires. I
also added a capactor connecting the two lines, making sure the direction was
correct, as recommended by Adafruit's NeoPixel Uberguide. I also added a
resistor on the data line from the Arduino.

Programming
-----------

The LEDs are hooked up serially, up then down then over. The FastLED library
just takes a serial index and turns on that LED, so I used a lookup table to
convert X,Y coordinates to that index. I wrote some Python code to output the
C++ lookup tables. I had previously used a [web page to generate the
tables](https://macetech.github.io/FastLED-XY-Map-Generator/) but wrote my own
script because I needed some other lookup tables. To compile, you'll need to
run the Python program and save the output as "offsets.hpp".

Demo
----

I wrote an SDL demo program to make testing animations easier. It's in the
demo/ folder. I originally had it including the animations from the Arduino
sketch and having an interface to make them both work, but that was cumbersome.
Now I just copy the code over and change whatever it needs. Press left and
right arrow keys to switch between animations.

Video
-----

I wanted to show short video clips in glorious 10x10 resolution, so I have a
script that converts a video file into an array of color values. This takes a
lot of space on an Arduino, so I'm planning on adding an SD card later.
