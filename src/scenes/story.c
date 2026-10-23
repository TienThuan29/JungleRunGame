#include "scenes/scene.h"
#include "core/res.h"

SceneId story_run(App* app) {
	SDL_Renderer* r = app->renderer;
	const SDL_Rect rightRect = { 714, 481, 73, 55 }, leftRect = { 206, 480, 74, 55 };
	const SDL_Rect fixScreen = { -70, 0, 1110, 668 };
	SDL_Texture* page[2] = {
		res_texture(r, "Resource\\Menu\\Story\\story1.bmp", 0),
		res_texture(r, "Resource\\Menu\\Story\\story2.bmp", 0),
	};
	SDL_Texture* red = res_texture(r, "Resource\\Menu\\Story\\red.bmp", 0);
	SDL_Texture* green = res_texture(r, "Resource\\Menu\\Story\\green.bmp", 0);
	TTF_Font* font = res_font("Resource\\Fonts\\Kanit-Light.ttf", 24);
	SDL_Texture* back = res_text(r, font, "Return", (SDL_Color){ 0, 0, 0, 255 });
	SDL_Texture* backHot = res_text(r, font, "Return", (SDL_Color){ 0, 255, 255, 255 });
	res_close_font(&font);
	SDL_Rect returnRect = { 450, 503, 0, 0 };
	ui_size(back, &returnRect);

	SceneId next = SCENE_STORY;
	if (!page[0] || !page[1] || !red || !green || !back) {
		app_fatal(app, "Missing story resources (see console)");
		next = SCENE_QUIT;
	}
	int current = 0, hotRight = 0, hotLeft = 0, hotBack = 0;
	while (next == SCENE_STORY) {
		SDL_Event e;
		while (next == SCENE_STORY && SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				next = SCENE_QUIT;
			}
			else if (e.type == SDL_MOUSEMOTION) {
				hotRight = ui_hit(&rightRect, e.motion.x, e.motion.y);
				hotLeft = ui_hit(&leftRect, e.motion.x, e.motion.y);
				hotBack = ui_hit(&returnRect, e.motion.x, e.motion.y);
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN) {
				int x = e.button.x, y = e.button.y;
				if (ui_hit(&rightRect, x, y)) {
					Mix_PlayChannel(-1, app->click, 0);
					current = 1;
				}
				if (ui_hit(&leftRect, x, y)) {
					Mix_PlayChannel(-1, app->click, 0);
					current = 0;
				}
				if (ui_hit(&returnRect, x, y)) {
					Mix_PlayChannel(-1, app->click, 0);
					next = SCENE_MENU;
				}
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				next = SCENE_MENU;
			}
		}
		SDL_RenderClear(r);
		SDL_RenderCopy(r, page[current], NULL, &fixScreen);
		if (current == 0) { // page 1: "next" arrow on the right
			SDL_RenderCopyEx(r, hotRight ? red : green, NULL, &rightRect, 0, NULL, hotRight ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
		}
		else {              // page 2: "back" arrow on the left
			SDL_RenderCopyEx(r, hotLeft ? red : green, NULL, &leftRect, 0, NULL, hotLeft ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
		}
		SDL_RenderCopy(r, hotBack ? backHot : back, NULL, &returnRect);
		SDL_RenderPresent(r);
		app_tick(app);
	}
	res_destroy_seq(page, 2);
	res_destroy_texture(&red);
	res_destroy_texture(&green);
	res_destroy_texture(&back);
	res_destroy_texture(&backHot);
	return next;
}
