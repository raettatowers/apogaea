#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <endian.h>

#include "constants.hpp"
#include "movies.hpp"

extern CRGB leds[LED_COUNT];
const char* const DIRECTORY = "animations";

MoviePlayer::MoviePlayer() :
  _initialized(false),
  _playing(false),
  _directory(),
  _file(),
  _currentFileIndex(0)
{
  // TODO: I think 5 might be the pin? Need to check if this is right
  if (!SD.begin(5)) {
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
  _initialized = true;
}

void MoviePlayer::next(char* const output, const int length) {
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

void MoviePlayer::previous(char* const output, const int length) {
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

int MoviePlayer::animate(uint8_t) {
  const int HEADER_OFFSET = 2;
  if (!_playing) {
    return 100;
  }

  if (!_file.available()) {
    _file.seek(0);
  }
  // Looks like the struct is packed
  static_assert(sizeof(leds) == LED_COUNT * 3);
  // TODO: Should I check the response number of bytes read?
  _file.read(reinterpret_cast<uint8_t*>(leds), sizeof(leds));
  return _millisPerFrame;
}
