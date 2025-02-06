#!/bin/sh
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32da voltageMonitor.ino $@
