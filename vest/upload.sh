#!/bin/sh
arduino-cli compile --fqbn esp32:esp32:d1_mini32 piddle.ino $@
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:d1_mini32 piddle.ino $@
