#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <cmath>
#include <cassert>

#define COUNT_OF(x) (sizeof((x)) / sizeof(0[(x)]))


const int WIDTH = 640;
const int HEIGHT = 480;

constexpr Sint16 widthFraction(uint8_t percent) {
  return static_cast<Sint16>(static_cast<float>(percent) / 100.0f * static_cast<float>(WIDTH));
}

constexpr Sint16 heightFraction(uint8_t percent) {
  return static_cast<Sint16>(static_cast<float>(percent) / 100.0f * static_cast<float>(HEIGHT));
}

#define wf(x) widthFraction((x))
#define hf(x) heightFraction((x))
void drawVest(SDL_Renderer *renderer) {
  const Uint32 color = 0xffffffff;

  // Draw the left side
  const Sint16 leftSideXs[] = {
    wf(20), wf(25), wf(30), wf(30), wf(15), wf(15),
  };
  const Sint16 leftSideYs[] = {
    hf(30), hf(30), hf(40), hf(70), hf(70), hf(40),
  };
  static_assert(COUNT_OF(leftSideXs) == COUNT_OF(leftSideYs));
  polygonColor(renderer, leftSideXs, leftSideYs, COUNT_OF(leftSideXs), color);

  // Draw the right side
  Sint16 rightSideXs[COUNT_OF(leftSideXs)];
  for (unsigned int i = 0; i < COUNT_OF(leftSideXs); ++i) {
    rightSideXs[i] = WIDTH - leftSideXs[i];
  }
  polygonColor(renderer, rightSideXs, leftSideYs, COUNT_OF(rightSideXs), color);

  // Draw the middle
  Sint16 middleXs[] = {
    wf(
  };
}
#undef wf
#undef hf


int main(int argc, char *argv[]) {

  // Returns zero on success else non-zero
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    printf("error initializing SDL: %s\n", SDL_GetError());
  }
  SDL_Window *win = SDL_CreateWindow("vest", SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

  SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

  bool shouldClose = false;

  int xSpeed = 5;
  int ySpeed = 3;
  const int RADIUS = 60;
  int objectX = RADIUS;
  int objectY = RADIUS;

  // Animation loop
  float angle_r = 0.0;
  while (!shouldClose) {
    SDL_Event event;

    // Events management
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        shouldClose = true;
        break;

      default:
        break;
      }
    }

    objectX += xSpeed;
    if (objectX - RADIUS < 0) {
      objectX = RADIUS;
      xSpeed = -xSpeed;
    } else if (objectX + RADIUS >= WIDTH) {
      objectX = WIDTH - RADIUS;
      xSpeed = -xSpeed;
    }

    objectY += ySpeed;
    if (objectY - RADIUS < 0) {
      objectY = RADIUS;
      ySpeed = -ySpeed;
    } else if (objectY + RADIUS >= HEIGHT) {
      objectY = HEIGHT - RADIUS;
      ySpeed = -ySpeed;
    }

    // Clears the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    drawVest(renderer);

    // Draw a circle
    const int red = static_cast<int>(sinf(angle_r) * 127 + 127);
    const int green = static_cast<int>(sinf(angle_r + M_PI / 3) * 127 + 127);
    const int blue = static_cast<int>(sinf(angle_r + M_PI * 2.0 / 3) * 127 + 127);
    assert(0 <= red && red <= 255);
    angle_r += 0.01;
    if (filledCircleRGBA(renderer, objectX, objectY, RADIUS, red, green, blue, 255) != 0) {
      printf("RGBA failed\n");
    }

    SDL_RenderPresent(renderer);

    // 60 FPS
    SDL_Delay(1000 / 60);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(win);
  SDL_Quit();

  return 0;
}
