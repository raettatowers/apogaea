Bicycle
=======

Paint
-----

I wanted to paint my bicycle. I liked the metallic color scheme that @FirePixie
added in [her
tutorial](https://learn.adafruit.com/playa-festival-bike/painting-decoration)
and I also liked this [fade
effect](https://www.youtube.com/watch?v=QchnDNSpi10) so I decided to try both.
Finally, I also really liked this [generative art bike paint
job](https://www.instructables.com/Turing-Pattern-Bike-Paint-Job/). My plan is
to have a chrome base, then have a fade effect between components.

I tried to remove the paint using paint thinner, but I guess they changed the
formula a few years ago and I had a lot of trople stripping. It took me a large
number of coats and a lot of rubbing with a steel wire brush, but I managed to
almost completely strip the fork. I decided to give up on the rest. Some people
recommend getting it sandblasted, but I didn't want to take the time.

I had some struggles getting the generative paint job code to work. For one
thing, the version of Inkscape in my distro was too old. I ended up installing
a newer Snap version. Snaps come with all requirements included, and that meant
it came with a specific version of Python for extensions. Unfortunately, that
Python installation didn't include some of the required libraries. I rewrote
part of it to remove that requirement, to fix a minor issue where selecting
muliple elements could cause a bug, and added some configuration options.
Here's my [modified generative bike Inkscape
plugin](https://gist.github.com/bskari/faf7cc90ca6979d12d857bb6107675c2).

I bought an old used Cricut Expression on Craigslist. Getting that thing to cut
arbitrary SVG files was a pain. There's some old software, Sure Cuts a Lot
version 2, that works. It doesn't seem to work on Windows 10, but it seems to
work on Windows XP VMs. Note that Cricut pushed a firmware update at some point
to disable compatibility with SCAL. I've read that you can downgrade the
firmware, but luckily, mine hadn't been updated yet.

### Measurements

- Front fork
  9.5 cm circumference, 35 cm length
- Head tube
  10.5 cm circumference, 7 cm with tubes, 11 cm length
- Top tube
  10.5 cm circumference, 52 cm length
- Bottom tube
  12 cm circumference, 58 cm length
- Seat tube
  9.5 cm circumference, 39 cm length
- Seat stay
  5 cm circumference, 39 cm length
- Chain stay
  6 cm circumference, 36 cm length
- Chain guard
  52 cm length total, 37 cm top to curve, 33 cm bottom to curve, 12 cm radius, 
