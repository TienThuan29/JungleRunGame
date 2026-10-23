#ifndef RES_H
#define RES_H
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

// Every texture/font/sound in the game is created and destroyed through these,
// so the Debug build can count what is alive.
SDL_Texture* res_texture(SDL_Renderer* r, const char* path, int keyBlack);
void res_texture_seq(SDL_Renderer* r, SDL_Texture* out[], int n, const char* fmt, int keyBlack); // fmt has %d, 1-based
SDL_Texture* res_text(SDL_Renderer* r, TTF_Font* font, const char* text, SDL_Color color);
void res_destroy_texture(SDL_Texture** t);
void res_destroy_seq(SDL_Texture* t[], int n);

TTF_Font* res_font(const char* path, int size);
void res_close_font(TTF_Font** f);

Mix_Chunk* res_wav(const char* path, int volume); // volume < 0: keep default
void res_free_wav(Mix_Chunk** c);
Mix_Music* res_mus(const char* path);
void res_free_mus(Mix_Music** m);

// Debug: "tex 42 font 3 wav 7 mus 2 | sdl_alloc 1234". Release: "".
void res_counts(char* buf, size_t len);
#endif
