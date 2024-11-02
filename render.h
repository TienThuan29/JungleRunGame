#ifndef RENDER
#define RENDER
#include "common_functions.h"
void do_render(GameState* gameState, int* done);
void render_character_animation(GameState* gameState, SDL_Texture* texture[], const int width, const int height, int frame);
void render_giga_golem(GameState* gameState, SDL_Texture* texture[]);
void render_status(GameState* gameState);
#endif // !RENDER

