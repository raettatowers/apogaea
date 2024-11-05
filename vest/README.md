LED vest
========

A glowing furry vest. Copied from an idea from Finchronicity.

- [Hackaday LED vest](https://hackaday.io/project/395-led-vest/log/570-power)
- [PJRC forums LED vest]([https://forum.pjrc.com/threads/25939-Led-vest)
- [Finchronicity's source code](https://github.com/finominal/LedVestPlasmaGenerator)

Version 2
=========

This is my second attempt at making a vest. Version 1 used LED strips. This
required a whole bunch of soldering: each end had to be soldered to the next,
and I had each strip connected in parallel to some wires running along
the bottom. Here's the problem: fabric likes to bend and stretch, but
solder joints and LED strips do not. Even with hot glue covering the
solder joints for strain relief, they were constantly being pulled
out. For version 2, I'm going to used seed LED strands which flex more and use
several of them connected to different pins, so that even if one connection
fails, the remaining ones will still work.

Parts
-----

- White shaggy faux fur fabric, Joann Fabrics, 1.5 yards: $48 https://www.joann.com/shaggy-fur-fabric/17763558.html
- Blank rug hooking mesh canvas kit: $18 https://www.amazon.com/gp/product/B07SFCYS5Y/
- Seed LED strand, 1000 count 30 mm per LED, WS2812B 5V white PCB IP65, Ray Wu's store Ali Express: $22 https://www.aliexpress.us/item/3256805296630315.html
- Arduino-compatible controller board, ESP32 Mini: $21 https://www.amazon.com/ESP32/dp/B07BK435ZW/
- Wire
- Soldering iron and solder
- Hot glue gun
- Capacitor, 500-1000 uF at 6.3V or higher
- Resistor, 300-500 ohm
- Sewing machine, thread, and hand needle

Sewing
------

I used the same pattern as my v1. This time, I didn't use a fabric liner.
Instead, I bought some a mesh rug making kit and threaded the cable through the
holes. I then used hook and loop to attach it to the fur vest part. This is
nice because I can remove the electric part from the fur for cleaning. Each
square in the mesh canvas is exactly 6 mm, so with my LED strand, I had 5
squares per LED.

Lighting
--------

I wound up with 298 active LEDs, a little more than v1. Due to how I wound the
LEDs, I wound up with some extra LEDs that won't be active and are skipped. In
total, about 350 LEDs.

Power
-----

Just from idling with no LEDs on, the LEDs and Arduino used 0.25 A at 5.1 V.
Each seed LED uses about 3.5 mA when displaying white at 25% brightness. When
displaying red, they use 1.5 mA. At 100% brightness, for white they use 12.5
mA, and for red they use 5 mA.

Similar to v1, I just used a USB battery pack. With 298 LEDs displaying white
at 100% brightness, this would require 3.7 A. This is much lower than the LED
strip version. The biggest USB battery pack I found can only output 2 A at 5 V,
so I'll still need to run it dimmed, or use FastLED's feature that limits
brightness to some max current.

Wiring
------

I wanted to use multiple strands, each connected to their own data line, so
that if one fails, the rest will still work. I weaved the LED strands through
the sqaure holes in the mesh canvas. I wanted to attach power to both ends of
the strand, both to reduce voltage drop, and to have a backup if the first
connection comes loose. I had the the Arduino on the left front side of the
vest, so all of the strands had to start there. This made planning a bit
complex: for the other side of the vest, I had to run a strand across the back
to the other side, then back again. I wanted each pass to be used in the final
lighting, so this required some planning so I didn't end up with too little
space, or wasted LEDs. I suggest laying things out, double checking, and
planning carefully.

Threading the strips through is extremely tedious and slow. For each 5 mm LED,
I would go under one thread and then back over. I had to pull the full strand
through after each over/under loop. When I had to reverse direction and go back
down the next row or column, because there's 5 mm per LED, there's not enough
strand to move over and continue without "wasting" an LED. I would leave one
unused LED on the end. I found that going down 2 squares, then diagonally 1,
then over 4 and back up would use the correct amount of strand to line up well.

I strongly recommend making small marks on the mesh where each LED should go.
It's really easy to be off by one, and threading these is so tedious and slow
that you won't want to redo it. I would also count the number of LEDs needed
for a strand, find the middle point in the mesh, count that number of LEDs from
the roll (plus 5 for each end for safety), mark the middle LED from that
strand, and then start threading it from those two middles. This really reduces
the amount of time you're pulling strand.

For power distribution, I used a small protoboard with the positive and
negative ends wired in parallel.

Programming
-----------

Because I have 5 different strands that are woven all over the place, trying to
map each X,Y coordinate to a strand and offset was a pain. I wrote a quick
sketch using the excellent [http://remotexy.com/](RemoteXY) to help. RemoteXY
is a web interface that lets you design GUIs that can be controlled from your
smart phone over Bluetooth or WiFi. I can't recommend it enough - it's so quick
and easy to make an app with a couple sliders, text input, and buttons that
seamlessly connect wireless using your phone.

The first app I wrote would turn on one LED at a time. It would let you select
which strand and LED offset to turn on. It had a text input for the offset, and
a button to go to the next LED. I counted the LEDs came up with a quick format
that would specify runs in a row or column and skipped LEDs. Then I wrote a
Python program that would convert these into lookup tables in C.

Once I had those lookup tables, I modified my RemoteXY program to turn on one
row and column of the LEDs at a time. Two buttons would move to the next row or
column. This worked well enough that I could find when I messed up my
specification and go back, fix it, recompile and retest.

Movies
------

I wanted to be able to play movies and animations on the vest. You'll need to run my Python
formatter on a video file, then put the resulting file in a folder named "animations" on the root of
the SD card. The first byte is the width, then the next byte is the height, then the next 2 bytes
are the milliseconds per frame little endian, then the rest is GRB uint32s, arranged row by row? I
don't know yet.

Version 1
=========

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
