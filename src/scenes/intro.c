#include "scenes/scene.h"
#include "core/res.h"
#include "game/defs.h"
#include <string.h>

static const char* story[7] = {
	"\"...In 2016, Chris, who wanted to cure his beloved sister Claire's late-stage cancer, set out for the",
	"forest after obtaining the location from a lucky explorer who survived the forest. He hoped to find the",
	"treasure in the legend and use its power to save his sister. He crossed through dangerous creatures and",
	"reached the relic. He advanced deep into the relic, trying to hide and find a way to defeat the Golem.",
	"finally. The Golem chased after him and used its power to trap him in an endless loop of being pursued",
	"until death. Chris, with no weapons except for the sword he picked up from the relic, had to break the",
	"loop and defeat the Golem to reach the treasure.\""
};

// Typewriter intro. Draws incrementally on purpose: earlier lines stay on screen,
// each step only repaints from the current line down (same as the original).
SceneId intro_run(App* app) {
	SDL_Renderer* r = app->renderer;
	const SDL_Color white = { 255, 255, 255, 255 };
	app_resize(app, WIDTH_WINDOW, HEIGHT_WINDOW);

	Mix_Music* typing = res_mus("Resource\\Sound\\Sound Effects\\typing.mp3");
	Mix_PlayMusic(typing, -1);
	Mix_VolumeMusic(64);
	TTF_Font* font = res_font("Resource\\Fonts\\crazy-pixel.ttf", 80);
	SDL_Texture* skip = res_text(r, font, ">> Press [SPACE] to skip <<", white);
	res_close_font(&font);
	font = res_font("Resource\\Fonts\\Kanit-Light.ttf", 20);
	SDL_Texture* black = res_texture(r, "Resource\\Menu\\LoginImage\\bg.bmp", 0);
	SDL_Texture* whitebar = res_texture(r, "Resource\\Menu\\LoginImage\\whitebar.bmp", 0);
	SDL_Texture* blackbar = res_texture(r, "Resource\\Menu\\LoginImage\\blackbar.bmp", 0);
	SDL_Texture* line = NULL;

	SDL_Rect skipRect = { 290, 500, 0, 0 };
	ui_size(skip, &skipRect);
	SDL_Rect lineRect = { 120, 76, 0, 0 };
	SDL_Rect underline = { 100, 100, 15, 5 };
	SDL_Rect blackRect = { 0, 100, 1200, 800 };
	int charNumber = 0, lineNumber = 0;

	SceneId next = SCENE_INTRO;
	if (!font || !black || !whitebar || !blackbar) {
		app_fatal(app, "Missing intro resources (see console)");
		next = SCENE_QUIT;
	}
	SDL_RenderClear(r);
	while (next == SCENE_INTRO) {
		SDL_Event e;
		while (next == SCENE_INTRO && SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) next = SCENE_QUIT;
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) next = SCENE_MENU;
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) next = SCENE_GAMEPLAY;
		}
		if (next != SCENE_INTRO) break;
		if (charNumber >= (int)strlen(story[lineNumber])) {
			lineNumber++;
			if (lineNumber >= 7) { // was an out-of-bounds read of story[7]
				next = SCENE_GAMEPLAY;
				break;
			}
			charNumber = 0;
			underline.y += 40;
			underline.x = 100;
			blackRect.y += 40;
			lineRect.y += 40;
		}
		charNumber += 5;
		char prefix[150];
		SDL_strlcpy(prefix, story[lineNumber], SDL_min(sizeof prefix, (size_t)charNumber + 1));
		res_destroy_texture(&line); // leak #5 fix (and old lineTexture[2] overflow)
		line = res_text(r, font, prefix, white);
		ui_size(line, &lineRect);
		underline.x += 44;

		SDL_Delay(100);
		SDL_RenderCopy(r, black, NULL, &blackRect);
		SDL_RenderCopy(r, line, NULL, &lineRect);
		SDL_RenderCopy(r, skip, NULL, &skipRect);
		SDL_RenderPresent(r);
		SDL_Delay(60);
		SDL_RenderCopy(r, underline.x >= 1000 ? blackbar : whitebar, NULL, &underline);
		SDL_RenderPresent(r);
		app_tick(app);
	}
	Mix_HaltMusic();
	res_free_mus(&typing);
	res_close_font(&font);
	res_destroy_texture(&skip);
	res_destroy_texture(&black);
	res_destroy_texture(&whitebar);
	res_destroy_texture(&blackbar);
	res_destroy_texture(&line);
	return next;
}
