#include "animations.hpp"
#include <FS.h>

class MoviePlayer : public Animation {
  public:
    MoviePlayer();
    ~MoviePlayer() = default;
    void next(char* output, int length);
    void previous(char* output, int length);
    void play();
    void pause();
    void togglePlay();
    operator bool() const;

    int animate(uint8_t) override;
    void reset() override;

  private:
    MoviePlayer(const MoviePlayer&) = delete;
    MoviePlayer(MoviePlayer&&) = delete;

    bool _initialized;
    bool _playing;
    File _directory;
    File _file;
    int _currentFileIndex;
    uint16_t _millisPerFrame;
};
