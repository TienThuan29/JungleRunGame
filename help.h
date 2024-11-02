#ifndef HELP
#define HELP
#include <SDL.h>
#include "structs.h"

void getHealthSurface(SDL_Surface* images[]);
void getGoodSurface(SDL_Surface* images[]);
void getRunningSurface(SDL_Surface* images[]);
void getJumpingSurface(SDL_Surface* images[]);
void clearSurface(SDL_Surface* surface[], int num);
void getTexture(SDL_Renderer* renderer, SDL_Texture* imageTexture[], SDL_Surface* imageSurface[], int num);
void Render(SDL_Renderer* renderer);
void helpDisplay(SDL_Window* window, SDL_Renderer* renderer, GameState* gameState);

#endif // !D_HELP
