#!/bin/sh
arduino-cli compile --fqbn esp32:esp32:esp32da voltageMonitor.ino $@ --warnings all
