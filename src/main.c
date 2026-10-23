#include "core/app.h"
#include "core/res.h"
#include "scenes/scene.h"

#ifdef _DEBUG
static const char* scene_name(SceneId s) {
	static const char* names[] = { "QUIT", "MENU", "STORY", "HELP", "DONATE", "GAMEMODE", "INTRO", "GAMEPLAY" };
	return names[s];
}
#endif

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;
	App app;
	if (!app_init(&app)) {
		app_quit(&app);
		return 1;
	}
	SceneId scene = SCENE_MENU;
	while (scene != SCENE_QUIT) {
#ifdef _DEBUG
		SceneId prev = scene;
#endif
		switch (scene) {
		case SCENE_MENU:     scene = menu_run(&app);     break;
		case SCENE_STORY:    scene = story_run(&app);    break;
		case SCENE_HELP:     scene = help_run(&app);     break;
		case SCENE_DONATE:   scene = donate_run(&app);   break;
		case SCENE_GAMEMODE: scene = gamemode_run(&app); break;
		case SCENE_INTRO:    scene = intro_run(&app);    break;
		case SCENE_GAMEPLAY: scene = gameplay_run(&app); break;
		default:             scene = SCENE_QUIT;         break;
		}
#ifdef _DEBUG
		char counts[128];
		res_counts(counts, sizeof counts);
		SDL_Log("[scene %s -> %s] %s", scene_name(prev), scene_name(scene), counts);
#endif
	}
	app_quit(&app);
	return 0;
}
