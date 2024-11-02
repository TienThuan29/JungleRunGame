#ifndef PREGAMEPLAY
#define PREGAMEPLAY
#include <SDL.h>
#include <SDL_ttf.h>
#include "structs.h"

void RenderUnderline(SDL_Renderer* renderer, int lineNumber);
void UpdateText(SDL_Renderer* renderer, int charNumber, int lineNumber);
void PreGameplay(GameState* gameState);

#endif // !PREGAMEPLAY
