#include "core/app.h"
#include "core/res.h"
#include "../../defs.h"
#include <SDL_image.h>
#include <SDL_ttf.h>

void app_fatal(App* app, const char* msg) {
	SDL_Log("fatal: %s", msg);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Jungle Run", msg, app ? app->window : NULL);
}

int app_init(App* app) {
	SDL_memset(app, 0, sizeof *app);
	app->difficulty = DIFFICULTY_EASY;
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
		app_fatal(NULL, SDL_GetError());
		return 0;
	}
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	Mix_Init(MIX_INIT_MP3);
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048) != 0) {
		SDL_Log("audio: %s", SDL_GetError()); // play without sound
	}
	app->window = SDL_CreateWindow("Jungle Run", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MENU_W, MENU_H, 0);
	if (!app->window) {
		app_fatal(NULL, SDL_GetError());
		return 0;
	}
	app->renderer = SDL_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!app->renderer) {
		app_fatal(app, SDL_GetError());
		return 0;
	}
	app->click = res_wav("Resource\\Sound\\Sound Effects\\click.wav", -1);
	app->menuMusic = res_mus("Resource\\Sound\\Soundtrack\\MainMenu2.mp3");
	return 1;
}

void app_quit(App* app) {
	Mix_HaltMusic();
	res_free_mus(&app->menuMusic);
	res_free_wav(&app->click);
	if (app->renderer) SDL_DestroyRenderer(app->renderer);
	if (app->window) SDL_DestroyWindow(app->window);
	Mix_CloseAudio();
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void app_resize(App* app, int w, int h) {
	int cw, ch;
	SDL_GetWindowSize(app->window, &cw, &ch);
	if (cw == w && ch == h) return;
	SDL_SetWindowSize(app->window, w, h);
	SDL_SetWindowPosition(app->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void app_tick(App* app) {
#ifdef _DEBUG
	static Uint32 last;
	Uint32 now = SDL_GetTicks();
	if (now - last < 1000) return;
	last = now;
	char counts[128], title[160];
	res_counts(counts, sizeof counts);
	SDL_snprintf(title, sizeof title, "Jungle Run [%s]", counts);
	SDL_SetWindowTitle(app->window, title);
#else
	(void)app;
#endif
}
