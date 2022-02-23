#include <cstdint>
using std::uint8_t;

enum class Tetrad {I, T, Z, S, O, L, J, MAX = J};

// Copied from https://meatfighter.com/nintendotetrisai/#Representing_Tetriminos
// { { X0, Y0 }, { X1, Y1 }, { X2, Y2 }, { X3, Y3 }, }
static constexpr int8_t TBlock1[4][2] = {
  { -1, 0}, {0, 0}, {1, 0}, {0, 1}, // 00: T up
};
static constexpr int8_t TBlock2[4][2] = {
  {0, 1}, {0, 0}, {1, 0}, {0, 1},  // 01: T right
};
static constexpr int8_t TBlock3[4][2] = {
  { -1, 0}, {0, 0}, {1, 0}, {0, 1}, // 02: T down (spawn)
};
static constexpr int8_t TBlock4[4][2] = {
  {0, 1}, { -1, 0}, {0, 0}, {0, 1}, // 03: T left
};
static constexpr int8_t JBlock1[4][2] = {
  {0, 1}, {0, 0}, { -1, 1}, {0, 1}, // 04: J left
};
static constexpr int8_t JBlock2[4][2] = {
  { -1, 1}, { -1, 0}, {0, 0}, {1, 0}, // 05: J up
};
static constexpr int8_t JBlock3[4][2] = {
  {0, 1}, {1, 1}, {0, 0}, {0, 1},  // 06: J right
};
static constexpr int8_t JBlock4[4][2] = {
  { -1, 0}, {0, 0}, {1, 0}, {1, 1}, // 07: J down (spawn)
};
static constexpr int8_t ZBlock1[4][2] = {
  { -1, 0}, {0, 0}, {0, 1}, {1, 1}, // 08: Z horizontal (spawn)
};
static constexpr int8_t ZBlock2[4][2] = {
  {1, 1}, {0, 0}, {1, 0}, {0, 1},  // 09: Z vertical
};
static constexpr int8_t OBlock1[4][2] = {
  { -1, 0}, {0, 0}, { -1, 1}, {0, 1}, // 0A: O (spawn)
};
static constexpr int8_t SBlock1[4][2] = {
  {0, 0}, {1, 0}, { -1, 1}, {0, 1}, // 0B: S horizontal (spawn)
};
static constexpr int8_t SBlock2[4][2] = {
  {0, 1}, {0, 0}, {1, 0}, {1, 1},  // 0C: S vertical
};
static constexpr int8_t LBlock1[4][2] = {
  {0, 1}, {0, 0}, {0, 1}, {1, 1},  // 0D: L right
};
static constexpr int8_t LBlock2[4][2] = {
  { -1, 0}, {0, 0}, {1, 0}, { -1, 1}, // 0E: L down (spawn)
};
static constexpr int8_t LBlock3[4][2] = {
  { -1, 1}, {0, 1}, {0, 0}, {0, 1}, // 0F: L left
};
static constexpr int8_t LBlock4[4][2] = {
  {1, 1}, { -1, 0}, {0, 0}, {1, 0}, // 10: L up
};
static constexpr int8_t IBlock1[4][2] = {
  {0, 2}, {0, 1}, {0, 0}, {0, 1},  // 11: I vertical
};
static constexpr int8_t IBlock2[4][2] = {
  { -2, 0}, { -1, 0}, {0, 0}, {1, 0}, // 12: I horizontal
};

typedef int8_t Block_t[4][2];

static const Block_t* const tetradRotationToBlock[7][4] {
  {
    &IBlock1,
    &IBlock2,
    &IBlock1,
    &IBlock2,
  },
  {
    &TBlock1,
    &TBlock2,
    &TBlock3,
    &TBlock4,
  },
  {
    &ZBlock1,
    &ZBlock2,
    &ZBlock1,
    &ZBlock2,
  },
  {
    &SBlock1,
    &SBlock2,
    &SBlock1,
    &SBlock2,
  },
  {
    &OBlock1,
    &OBlock1,
    &OBlock1,
    &OBlock1,
  },
  {
    &LBlock1,
    &LBlock2,
    &LBlock3,
    &LBlock4,
  },
  {
    &JBlock1,
    &JBlock2,
    &JBlock3,
    &JBlock4,
  },
};

class Tetris {
  public:
    Tetris();
    ~Tetris() = default;
    int update();

  private:
    void drop();
    bool droppable() const;
    void draw() const;
    void resetPlayfield();
    void reset();
    void spawnTetrad();
    bool gameOver() const;

    static const int HEIGHT = 11;
    // The full usable back width is 10, but the NES's 10x20 (plus 2
    // for spawning), so I made it slightly smaller to compensate
    static const int WIDTH = 8;
    static const int START_COLUMN = 10;

    // Use uint8_t so we can have hues
    uint8_t playField[HEIGHT][WIDTH];

    Tetrad tetrad;
    uint8_t rotation;
    uint8_t hue;
    int tetradX, tetradY;
};


Tetris::Tetris() : playField(), tetrad(Tetrad::L), rotation(0), tetradX(WIDTH / 2), tetradY(HEIGHT - 2)
{
  resetPlayfield();
}

void Tetris::reset() {
  resetPlayfield();
}

void Tetris::resetPlayfield() {
  for (int row = 0; row < HEIGHT; ++row) {
    for (int column = 0; column < WIDTH; ++column) {
      playField[row][column] = 0;
    }
  }
}

void Tetris::drop() {
  if (droppable()) {
    --tetradY;
  } else if (gameOver()) {
    // TODO: Do cool game over animation stuff
    reset();
  } else {
    // Copy the tetrad to the playfield
    for (int i = 0; i < 4; ++i) {
      const int tetradIndex = static_cast<int>(tetrad);
      const auto xOffset = *(tetradRotationToBlock[tetradIndex][rotation])[i][0];
      const auto yOffset = *(tetradRotationToBlock[tetradIndex][rotation])[i][1];
      playField[tetradY + yOffset][tetradX + xOffset] = hue;
      spawnTetrad();
    }
  }
}

void Tetris::spawnTetrad() {
  tetrad = Tetrad((static_cast<int>(tetrad) + 1) % static_cast<int>(Tetrad::MAX));
}

bool gameOver() {
  // TODO
  static int count = 0;
  ++count;
  if (count > 3) {
    return true;
  }
}

bool Tetris::droppable() const {
  for (int i = 0; i < 4; ++i) {
    const int tetradIndex = static_cast<int>(tetrad);
    const auto xOffset = *(tetradRotationToBlock[tetradIndex][rotation])[i][0];
    const auto yOffset = *(tetradRotationToBlock[tetradIndex][rotation])[i][1];
    if (tetradY - 1 + yOffset < 0) {
      return false;
    }

    if (playField[tetradY - 1 + yOffset][tetradX + xOffset] != 0) {
      return false;
    }
  }
  return true;
}
