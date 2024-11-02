#ifndef MENU
#define MENU
#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "help.h"
#include "preGameplay.h"
#include "checkAccount.h"
#include "structs.h"

void accountDisplay(GameState* gameState);
void loginDisplay(GameState* gameState);
void registerDisplay(GameState* gameState);
void QRcode(SDL_Window* window, SDL_Renderer* renderer, GameState* gameState);
void storyDisplay(SDL_Window* window, SDL_Renderer* renderer, GameState* gameState);
void gameMode(SDL_Window* window, SDL_Renderer* renderer, char selectMode[], int* ptrNumMode, GameState* gameState);

#endif // !MENU
