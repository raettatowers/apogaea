#!/bin/sh
arduino-cli compile --fqbn esp32:esp32:esp32da piddle.ino $@ #--warnings all
