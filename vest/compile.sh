#!/bin/sh
arduino-cli compile --fqbn esp32:esp32:d1_mini32 vest.ino $@ --warnings all
