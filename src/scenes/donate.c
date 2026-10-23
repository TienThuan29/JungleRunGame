#include "scenes/scene.h"
#include "core/res.h"

SceneId donate_run(App* app) {
	SDL_Renderer* r = app->renderer;
	const SDL_Rect qrRect = { 398, 255, 170, 170 }, logoRect = { 328, 300, 60, 60 };
	SDL_Texture* bg = res_texture(r, "Resource\\Menu\\LoginImage\\donate.bmp", 0);
	SDL_Texture* qr = res_texture(r, "Resource\\Menu\\LoginImage\\QR.bmp", 0);
	SDL_Texture* logo = res_texture(r, "Resource\\Menu\\LoginImage\\momologo.bmp", 0);
	TTF_Font* font = res_font("Resource\\Fonts\\Kanit-Light.ttf", 24);
	SDL_Texture* back = res_text(r, font, "Return", (SDL_Color){ 0, 0, 0, 255 });
	SDL_Texture* backHot = res_text(r, font, "Return", (SDL_Color){ 0, 255, 255, 255 });
	res_close_font(&font);
	font = res_font("Resource\\Fonts\\Kanit-MediumItalic.ttf", 30);
	SDL_Texture* thanks = res_text(r, font, "^^Thank You For Your Donation^^", (SDL_Color){ 0, 255, 0, 255 });
	res_close_font(&font);
	SDL_Rect returnRect = { 447, 440, 0, 0 }, thankRect = { 247, 200, 0, 0 };
	ui_size(back, &returnRect);
	ui_size(thanks, &thankRect);

	SceneId next = SCENE_DONATE;
	if (!bg || !back) {
		app_fatal(app, "Missing donate resources (see console)");
		next = SCENE_QUIT;
	}
	int hotBack = 0;
	while (next == SCENE_DONATE) {
		SDL_Event e;
		while (next == SCENE_DONATE && SDL_PollEvent(&e)) {
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
		SDL_RenderClear(r);
		SDL_RenderCopy(r, bg, NULL, NULL);
		SDL_RenderCopy(r, qr, NULL, &qrRect);
		SDL_RenderCopy(r, logo, NULL, &logoRect);
		SDL_RenderCopy(r, thanks, NULL, &thankRect);
		SDL_RenderCopy(r, hotBack ? backHot : back, NULL, &returnRect);
		SDL_RenderPresent(r);
		app_tick(app);
	}
	res_destroy_texture(&bg);
	res_destroy_texture(&qr);
	res_destroy_texture(&logo);
	res_destroy_texture(&thanks);
	res_destroy_texture(&back);
	res_destroy_texture(&backHot);
	return next;
}
