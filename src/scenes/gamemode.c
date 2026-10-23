#include "scenes/scene.h"
#include "core/res.h"
#include "game/defs.h"

SceneId gamemode_run(App* app) {
	static const char* modes[3] = { "Easy", "Medium", "Hard" };
	static const int difficulty[3] = { DIFFICULTY_EASY, DIFFICULTY_MEDIUM, DIFFICULTY_HARD };
	static const SDL_Color modeColor[3] = { { 0, 255, 0, 255 }, { 255, 200, 0, 255 }, { 255, 0, 0, 255 } };
	static const int modeX[3] = { 435, 415, 435 };
	const SDL_Color black = { 0, 0, 0, 255 }, red = { 255, 0, 0, 255 };
	const SDL_Rect trickRect = { 390, 300, 190, 70 };
	SDL_Renderer* r = app->renderer;

	SDL_Texture* bg = res_texture(r, "Resource\\Menu\\LoginImage\\gamemode.bmp", 0);
	SDL_Texture* trick = res_texture(r, "Resource\\Menu\\LoginImage\\trick3.bmp", 0);
	TTF_Font* font = res_font("Resource\\Fonts\\Kanit-Light.ttf", 24);
	SDL_Texture* back = res_text(r, font, "Return", black);
	SDL_Texture* backHot = res_text(r, font, "Return", (SDL_Color){ 0, 255, 255, 255 });
	res_close_font(&font);
	font = res_font("Resource\\Fonts\\Kanit-Medium.ttf", 60);
	SDL_Texture* arrow[2] = { res_text(r, font, "<<", black), res_text(r, font, ">>", black) };
	SDL_Texture* arrowHot[2] = { res_text(r, font, "<<", red), res_text(r, font, ">>", red) };
	res_close_font(&font);
	font = res_font("Resource\\Fonts\\crazy-pixel.ttf", 100);
	SDL_Texture* title = res_text(r, font, "Game Mode", (SDL_Color){ 0, 200, 255, 255 });
	res_close_font(&font);
	TTF_Font* modeFont = res_font("Resource\\Fonts\\crazy-pixel.ttf", 80);
	SDL_Texture* mode = res_text(r, modeFont, modes[app->modeIndex], modeColor[app->modeIndex]);

	SDL_Rect returnRect = { 446, 439, 0, 0 }, titleRect = { 350, 130, 0, 0 };
	SDL_Rect arrowRect[2] = { { 310, 290, 0, 0 }, { 595, 290, 0, 0 } };
	ui_size(back, &returnRect);
	ui_size(title, &titleRect);
	ui_size(arrow[0], &arrowRect[0]);
	ui_size(arrow[1], &arrowRect[1]);

	SceneId next = SCENE_GAMEMODE;
	if (!bg || !back || !modeFont) {
		app_fatal(app, "Missing game mode resources (see console)");
		next = SCENE_QUIT;
	}
	int hotBack = 0, hotArrow[2] = { 0, 0 };
	while (next == SCENE_GAMEMODE) {
		SDL_Event e;
		while (next == SCENE_GAMEMODE && SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				next = SCENE_QUIT;
			}
			else if (e.type == SDL_MOUSEMOTION) {
				hotBack = ui_hit(&returnRect, e.motion.x, e.motion.y);
				for (int i = 0; i < 2; i++) hotArrow[i] = ui_hit(&arrowRect[i], e.motion.x, e.motion.y);
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN) {
				int x = e.button.x, y = e.button.y;
				if (ui_hit(&returnRect, x, y)) {
					Mix_PlayChannel(-1, app->click, 0);
					next = SCENE_MENU;
				}
				int step = ui_hit(&arrowRect[0], x, y) ? 2 : ui_hit(&arrowRect[1], x, y) ? 1 : 0; // 2 == -1 mod 3
				if (step) {
					app->modeIndex = (app->modeIndex + step) % 3;
					app->difficulty = difficulty[app->modeIndex];
					res_destroy_texture(&mode);
					mode = res_text(r, modeFont, modes[app->modeIndex], modeColor[app->modeIndex]);
				}
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				next = SCENE_MENU;
			}
		}
		SDL_Rect modeRect = { modeX[app->modeIndex], 265, 0, 0 };
		ui_size(mode, &modeRect);
		SDL_RenderClear(r);
		SDL_RenderCopy(r, bg, NULL, NULL);
		SDL_RenderCopy(r, trick, NULL, &trickRect);
		SDL_RenderCopy(r, hotBack ? backHot : back, NULL, &returnRect);
		for (int i = 0; i < 2; i++) {
			SDL_RenderCopy(r, hotArrow[i] ? arrowHot[i] : arrow[i], NULL, &arrowRect[i]);
		}
		SDL_RenderCopy(r, title, NULL, &titleRect);
		SDL_RenderCopy(r, mode, NULL, &modeRect);
		SDL_RenderPresent(r);
		app_tick(app);
	}
	res_destroy_texture(&bg);
	res_destroy_texture(&trick);
	res_destroy_texture(&back);
	res_destroy_texture(&backHot);
	res_destroy_seq(arrow, 2);
	res_destroy_seq(arrowHot, 2);
	res_destroy_texture(&title);
	res_destroy_texture(&mode);
	res_close_font(&modeFont);
	return next;
}
