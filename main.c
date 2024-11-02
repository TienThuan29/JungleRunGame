#include <stdio.h>
#include <stdlib.h>
#include "menu.h"
#include "common_functions.h"

int main(int argc, char* argv[]) {
	// menu
	GameState gameState;
	gameState.setDifficulty = 0;
	SDL_Window* window = NULL; // declare a window
	SDL_Surface* screen = NULL;
	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow("Jungle Run", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
	Mix_Music* music = Mix_LoadMUS("Resource\\Sound\\Soundtrack\\MainMenu2.mp3");
	Mix_VolumeMusic(32);
	Mix_PlayMusic(music, -1);

	screen = SDL_GetWindowSurface(window);

	SDL_UpdateWindowSurface(window);

	SDL_Event e;

	bool quit = false;

	while (!quit) {
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) { quit = true; }
		}

		SDL_DestroyWindow(window);

		loginDisplay(&gameState);

		quit = true;
	}
	TTF_Quit();
	Mix_Quit();
	SDL_Quit();
	return 0;
}