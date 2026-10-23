#ifndef SCENE_H
#define SCENE_H
#include "core/app.h"

typedef enum {
	SCENE_QUIT, SCENE_MENU, SCENE_STORY, SCENE_HELP, SCENE_DONATE, SCENE_GAMEMODE, SCENE_INTRO, SCENE_GAMEPLAY
} SceneId;

// Each scene owns what it loads, frees it before returning, and returns the next scene.
// No scene calls another scene.
SceneId menu_run(App* app);
SceneId story_run(App* app);
SceneId help_run(App* app);
SceneId donate_run(App* app);
SceneId gamemode_run(App* app);
SceneId intro_run(App* app);
SceneId gameplay_run(App* app);

static inline int ui_hit(const SDL_Rect* r, int x, int y) {
	return x >= r->x && x <= r->x + r->w && y >= r->y && y <= r->y + r->h;
}

static inline void ui_size(SDL_Texture* t, SDL_Rect* r) {
	SDL_QueryTexture(t, NULL, NULL, &r->w, &r->h);
}
#endif
