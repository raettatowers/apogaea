#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <cmath>
#include <cassert>

int main(int argc, char *argv[]) {

  // Returns zero on success else non-zero
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    printf("error initializing SDL: %s\n", SDL_GetError());
  }
  // Creates a window
  const int WIDTH = 640;
  const int HEIGHT = 480;
  SDL_Window *win = SDL_CreateWindow("vest", SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

  // Triggers the program that controls
  // your graphics hardware and sets flags
  Uint32 render_flags = SDL_RENDERER_ACCELERATED;

  // Creates a renderer to render our images
  SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, render_flags);

  // Controls animation loop, can't use the name "close" because it's used by stdio
  int shouldClose = 0;

  // Box properties
  int xSpeed = 5;
  int ySpeed = 3;
  int boxX = 0;
  int boxY = 0;
  const int boxWidth = 60;
  const int boxHeight = 60;

  // Animation loop
  float angle_r = 0.0;
  while (!shouldClose) {
    SDL_Event event;

    // Events management
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        // Handling of close button
        shouldClose = 1;
        break;

      default:
        break;
      }
    }

    boxX += xSpeed;
    if (boxX < 0) {
      boxX = 0;
      xSpeed = -xSpeed;
    } else if (boxX + boxWidth >= WIDTH) {
      boxX = WIDTH - boxWidth;
      xSpeed = -xSpeed;
    }

    boxY += ySpeed;
    if (boxY < 0) {
      boxY = 0;
      ySpeed = -ySpeed;
    } else if (boxY + boxHeight >= HEIGHT) {
      boxY = HEIGHT - boxHeight;
      ySpeed = -ySpeed;
    }

    // Clears the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw a circle
    const int red = static_cast<int>(sinf(angle_r) * 127 + 127);
    const int green = static_cast<int>(sinf(angle_r + M_PI / 3) * 127 + 127);
    const int blue = static_cast<int>(sinf(angle_r + M_PI * 2.0 / 3) * 127 + 127);
    assert(0 <= red && red <= 255);
    angle_r += 0.01;
    if (boxRGBA(renderer, boxX, boxY, boxX + boxWidth, boxY + boxWidth, red, green, blue, 255) != 0) {
      printf("boxRGBA failed\n");
    }

    // Triggers the double buffers
    // for multiple rendering
    SDL_RenderPresent(renderer);

    // calculates to 60 fps
    SDL_Delay(1000 / 60);
  }

  // Destroy renderer
  SDL_DestroyRenderer(renderer);

  // Destroy window
  SDL_DestroyWindow(win);

  // Close SDL
  SDL_Quit();

  return 0;
}
