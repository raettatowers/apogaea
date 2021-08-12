#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <cassert>
#include <cmath>
#include <cstdint>

#include "../animations.hpp"
#include "../constants.hpp"

const int WIDTH = 640;
const int HEIGHT = 480;

SDL_Renderer *renderer = nullptr;

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
  if (polygonColor(renderer, leftSideXs, leftSideYs, COUNT_OF(leftSideXs), color) != 0) {
    fprintf(stderr, "polygonColor failed\n");
  }

  // Draw the right side
  Sint16 rightSideXs[COUNT_OF(leftSideXs)];
  for (unsigned int i = 0; i < COUNT_OF(leftSideXs); ++i) {
    rightSideXs[i] = WIDTH - leftSideXs[i];
  }
  if (polygonColor(renderer, rightSideXs, leftSideYs, COUNT_OF(rightSideXs), color) != 0) {
    fprintf(stderr, "polygonColor failed\n");
  }

  // Draw the middle
  Sint16 middleXs[] = {
    wf(30), wf(35), wf(65), wf(70), wf(70), wf(30)
  };
  Sint16 middleYs[] = {
    hf(40), hf(30), hf(30), hf(40), hf(70), hf(70)
  };
  static_assert(COUNT_OF(middleXs) == COUNT_OF(middleYs));
  if (polygonColor(renderer, middleXs, middleYs, COUNT_OF(middleXs), color) != 0) {
    fprintf(stderr, "polygonColor failed\n");
  }
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

  renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

  bool shouldClose = false;
  Ripple ripple;

  // Animation loop
  while (!shouldClose) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    drawVest(renderer);
    int delay_ms = ripple.animate(0xFF0000FF);
    SDL_RenderPresent(renderer);

    while (delay_ms > 0) {
      // 60 FPS
      SDL_Delay(1000 / 60);
      delay_ms -= 60;

      SDL_Event event;
      // Events management
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
          shouldClose = true;
          goto exitDelay;

        default:
          break;
        }
      }
    }
  }
exitDelay:

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(win);
  SDL_Quit();

  return 0;
}
