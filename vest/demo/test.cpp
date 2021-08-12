#include <SDL2/SDL.h>

class Framework {
public:
  // Contructor which initialize the parameters.
  Framework(int height_, int width_)
      : height(height_), width(width_), renderer(), window() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
  }

  // Destructor
  ~Framework() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
  }

private:
  const int height;
  const int width;
  SDL_Renderer *renderer;
  SDL_Window *window;
};


int main(int argc, char *argv[]) {
  // Creating the object by passing Height and Width value.
  Framework fw(200, 400);

  SDL_Event event;

  while (!(event.type == SDL_QUIT)) {
    SDL_Delay(1000 / 60); // 60 FPS
    SDL_PollEvent(&event);
  }
}
