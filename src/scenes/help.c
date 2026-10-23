#include "scenes/scene.h"
#include "core/res.h"

SceneId help_run(App* app) {
	SDL_Renderer* r = app->renderer;
	const SDL_Rect healthRect = { 550, 302, 40, 40 }, goodRect = { 460, 365, 40, 40 };
	const SDL_Rect runningRect = { 500, 130, 35, 65 }, jumpingRect = { 430, 214, 35, 65 };
	const SDL_Rect ceilingRect = { 610, 410, 60, 50 }, fireRect = { 670, 370, 55, 90 };
	const SDL_Rect sawRect = { 720, 410, 50, 50 }, groundRect = { 750, 410, 70, 50 };
	const SDL_Rect lightningRect = { 793, 400, 70, 60 };

	SDL_Texture* bg = res_texture(r, "Resource\\Menu\\LoginImage\\help.bmp", 0);
	SDL_Texture* fire = res_texture(r, "Resource\\Menu\\traps\\fire.bmp", 0);
	SDL_Texture* saw = res_texture(r, "Resource\\Menu\\traps\\saw.bmp", 0);
	SDL_Texture* ground = res_texture(r, "Resource\\Menu\\traps\\ground.bmp", 0);
	SDL_Texture* lightning = res_texture(r, "Resource\\Menu\\traps\\lightning.bmp", 0);
	SDL_Texture* ceiling = res_texture(r, "Resource\\Menu\\traps\\ceiling.bmp", 0);
	SDL_Texture* health[7], * good[8], * running[8], * jumping[5];
	res_texture_seq(r, health, 7, "Resource\\Menu\\item\\health%d.bmp", 1);
	res_texture_seq(r, good, 8, "Resource\\Menu\\item\\good%d.bmp", 1);
	res_texture_seq(r, running, 8, "Resource\\Menu\\running\\run%d.bmp", 0);
	res_texture_seq(r, jumping, 5, "Resource\\Menu\\jump\\jump%d.bmp", 0);
	TTF_Font* font = res_font("Resource\\Fonts\\Kanit-Light.ttf", 24);
	SDL_Texture* back = res_text(r, font, "Return", (SDL_Color){ 0, 0, 0, 255 });
	SDL_Texture* backHot = res_text(r, font, "Return", (SDL_Color){ 0, 255, 255, 255 });
	res_close_font(&font);
	SDL_Rect returnRect = { 446, 503, 0, 0 };
	ui_size(back, &returnRect);

	SceneId next = SCENE_HELP;
	if (!bg || !back) {
		app_fatal(app, "Missing help resources (see console)");
		next = SCENE_QUIT;
	}
	int hotBack = 0;
	while (next == SCENE_HELP) {
		SDL_Event e;
		while (next == SCENE_HELP && SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				next = SCENE_QUIT;
			}
			else if (e.type == SDL_MOUSEMOTION) {
				hotBack = ui_hit(&returnRect, e.motion.x, e.motion.y);
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN && ui_hit(&returnRect, e.button.x, e.button.y)) {
				Mix_PlayChannel(-1, app->click, 0);
				next = SCENE_MENU;
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				next = SCENE_MENU;
			}
		}
		Uint64 t = SDL_GetTicks64();
		SDL_RenderClear(r);
		SDL_RenderCopy(r, bg, NULL, NULL);
		SDL_RenderCopy(r, hotBack ? backHot : back, NULL, &returnRect);
		SDL_RenderCopy(r, lightning, NULL, &lightningRect);
		SDL_RenderCopy(r, ground, NULL, &groundRect);
		SDL_RenderCopy(r, fire, NULL, &fireRect);
		SDL_RenderCopy(r, saw, NULL, &sawRect);
		SDL_RenderCopy(r, health[t / 120 % 7], NULL, &healthRect);
		SDL_RenderCopy(r, good[t / 120 % 8], NULL, &goodRect);
		SDL_RenderCopy(r, running[t / 120 % 8], NULL, &runningRect);
		SDL_RenderCopy(r, jumping[t / 200 % 5], NULL, &jumpingRect);
		SDL_RenderCopy(r, ceiling, NULL, &ceilingRect);
		SDL_RenderPresent(r);
		app_tick(app);
	}
	res_destroy_texture(&bg);
	res_destroy_texture(&fire);
	res_destroy_texture(&saw);
	res_destroy_texture(&ground);
	res_destroy_texture(&lightning);
	res_destroy_texture(&ceiling);
	res_destroy_seq(health, 7);
	res_destroy_seq(good, 8);
	res_destroy_seq(running, 8);
	res_destroy_seq(jumping, 5);
	res_destroy_texture(&back);
	res_destroy_texture(&backHot);
	return next;
}
