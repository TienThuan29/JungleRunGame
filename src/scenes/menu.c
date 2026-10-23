#include "scenes/scene.h"
#include "core/res.h"

SceneId menu_run(App* app) {
	static const char* labels[6] = { "Single Player", "Story", "Game Mode", "Help", "Donate", "Exit" };
	static const SDL_Point at[6] = { { 400, 170 }, { 440, 225 }, { 410, 280 }, { 450, 335 }, { 430, 390 }, { 450, 445 } };
	static const SceneId target[6] = { SCENE_INTRO, SCENE_STORY, SCENE_GAMEMODE, SCENE_HELP, SCENE_DONATE, SCENE_QUIT };
	SDL_Renderer* r = app->renderer;
	app_resize(app, MENU_W, MENU_H);
	if (!Mix_PlayingMusic()) {
		Mix_VolumeMusic(32);
		Mix_PlayMusic(app->menuMusic, -1);
	}

	SDL_Texture* bg = res_texture(r, "Resource\\Menu\\LoginImage\\ChooseBG.bmp", 0);
	TTF_Font* font = res_font("Resource\\Fonts\\crazy-pixel.ttf", 50);
	SDL_Texture* normal[6], * hot[6];
	SDL_Rect pos[6];
	for (int i = 0; i < 6; i++) {
		normal[i] = res_text(r, font, labels[i], (SDL_Color){ 0, 0, 0, 255 });
		hot[i] = res_text(r, font, labels[i], (SDL_Color){ 255, 0, 0, 255 });
		pos[i] = (SDL_Rect){ at[i].x, at[i].y, 0, 0 };
		ui_size(normal[i], &pos[i]);
	}
	res_close_font(&font);

	SceneId next = SCENE_MENU;
	if (!bg || !normal[0]) {
		app_fatal(app, "Missing menu resources (see console)");
		next = SCENE_QUIT;
	}
	int hover = -1;
	while (next == SCENE_MENU) {
		SDL_Event e;
		while (next == SCENE_MENU && SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				next = SCENE_QUIT;
			}
			else if (e.type == SDL_MOUSEMOTION) {
				hover = -1;
				for (int i = 0; i < 6; i++) {
					if (ui_hit(&pos[i], e.motion.x, e.motion.y)) hover = i;
				}
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN) {
				for (int i = 0; i < 6; i++) {
					if (ui_hit(&pos[i], e.button.x, e.button.y)) {
						Mix_PlayChannel(-1, app->click, 0);
						next = target[i];
					}
				}
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				next = SCENE_QUIT;
			}
		}
		SDL_RenderClear(r);
		SDL_RenderCopy(r, bg, NULL, NULL);
		for (int i = 0; i < 6; i++) {
			SDL_RenderCopy(r, i == hover ? hot[i] : normal[i], NULL, &pos[i]);
		}
		SDL_RenderPresent(r);
		app_tick(app);
	}
	if (next == SCENE_INTRO) {
		Mix_HaltMusic();
	}
	res_destroy_seq(normal, 6);
	res_destroy_seq(hot, 6);
	res_destroy_texture(&bg);
	return next;
}
