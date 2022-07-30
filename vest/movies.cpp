#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <endian.h>

#include "constants.hpp"
#include "movies.hpp"

extern CRGB leds[LED_COUNT];
static const char* const DIRECTORY = "animations";

MoviePlayer::MoviePlayer() :
  _playing(false),
  _directory(),
  _file(),
  _currentFileIndex(0),
  _millisPerFrame()
{
  pinMode(SD_PIN, OUTPUT);
  if (!SD.begin(SD_PIN)) {
    return;
  }

  if (SD.cardType() == CARD_NONE) {
    return;
  }

  _directory = SD.open(DIRECTORY);
  if (!_directory.isDirectory()) {
    return;
  }

  _file = _directory.openNextFile();
  if (_file) {
    static_assert(BYTE_ORDER == LITTLE_ENDIAN);
    _file.read(reinterpret_cast<uint8_t*>(&_millisPerFrame), sizeof(_millisPerFrame));
  }
}

void MoviePlayer::next(char* const output, const size_t length) {
  _file = _directory.openNextFile();
  if (!_file) {
    _directory.rewindDirectory();
    _file = _directory.openNextFile();
  }
  const char* const name = _file.name();
  strncpy(output, name, length);
  output[length - 1] = '\0';

  ++_currentFileIndex;
  _playing = false;
}

void MoviePlayer::previous(char* const output, const size_t length) {
  if (_currentFileIndex > 0) {
    _directory.rewindDirectory();
    for (int i = 0; i <= _currentFileIndex; ++i) {
      _file = _directory.openNextFile();
      static_assert(BYTE_ORDER == LITTLE_ENDIAN);
      _file.read(reinterpret_cast<uint8_t*>(&_millisPerFrame), sizeof(_millisPerFrame));
    }
    --_currentFileIndex;
  } else {
    // TODO: Support previous when we're at the start
  }
  _playing = false;
}

void MoviePlayer::play() {
  _playing = true;
}

void MoviePlayer::pause() {
  _playing = false;
}

void MoviePlayer::togglePlay() {
  _playing = !_playing;
}

MoviePlayer::operator bool() const {
  return static_cast<bool>(_file);
}

int MoviePlayer::animate(uint8_t) {
  const int HEADER_OFFSET = 2;
  if (!_playing) {
    return 100;
  }

  if (!_file.available()) {
    _file.seek(sizeof(_millisPerFrame));
  }
  // Looks like the struct is packed
  static_assert(sizeof(leds) == LED_COUNT * 3);
  // TODO: Should I check the response number of bytes read?
  _file.read(reinterpret_cast<uint8_t*>(leds), sizeof(leds));
  return _millisPerFrame;
}


void MoviePlayer::reset() {
  _file.seek(sizeof(_millisPerFrame));
}
