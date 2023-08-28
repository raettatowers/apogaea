#!/bin/sh
arduino-cli compile --fqbn esp32:esp32:esp32doit-devkit-v1 piddle.ino $@
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32doit-devkit-v1 piddle.ino $@
