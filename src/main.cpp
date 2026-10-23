#include "core/Game.h"

int main(int, char*[]) {  // this exact signature: SDL2main.lib expects SDL_main(int, char**)
	jr::Game game;
	return game.ok() ? game.run() : 1;
}
