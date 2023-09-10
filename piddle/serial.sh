#!/bin/sh
# Dump the serial connection from Arduino
stty -F /dev/ttyUSB0 raw 115200
cat /dev/ttyUSB0
