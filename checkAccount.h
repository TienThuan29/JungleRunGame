#ifndef CHECKACCOUNT
#define CHECKACCOUNT
#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

int checkRepeatedUsername(char username[]);
int checkPassword(char username[], char pass[]);
void getText(SDL_Renderer* renderer,
    SDL_Surface* textSurface,
    SDL_Texture* textTexture,
    TTF_Font* font,
    char text[],
    int x, int y, SDL_Color color);
bool checkUsername(char username[]);
void writeIntoText(char username[], char password[]);
bool checkRepeatPassword(char pass[], char repeatPass[]);
#endif // !CHECKACCOUNT
