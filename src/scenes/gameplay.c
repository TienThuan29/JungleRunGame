#include "scenes/scene.h"
#include "game/game.h"
#include <math.h>

static int over_ready(const GameState* gs) {
	return gs->isOver && SDL_GetTicks64() / 1000.0f - gs->overTiming >= 2.0f;
}

// Was processEvents(): now also owns R/X on the ending screen (render used to poll too and lost keys).
static SceneId handle_input(GameState* gs) {
	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) {
		gs->player.status = 1;
		gs->player.flip = 1;
		gs->player.dx -= 0.3f;
		if (gs->player.dx < -2.5f) gs->player.dx = -2.5f;
	}
	else if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) {
		gs->player.status = 1;
		gs->player.flip = 0;
		gs->player.dx += 0.3f;
		if (gs->player.dx > 2.5f) gs->player.dx = 2.5f;
	}
	else if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S]) {
		gs->player.dy += 0.3f;
	}
	else {
		gs->player.status = 0;
		gs->player.flip = 0;
		gs->player.dx *= 0.8f;
		if (fabsf(gs->player.dx) < 0.1f) gs->player.dx = 0;
	}
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) return SCENE_QUIT;
		if (event.type != SDL_KEYDOWN) continue;
		switch (event.key.keysym.sym) {
		case SDLK_SPACE:
		case SDLK_UP:
		case SDLK_w:
			if (gs->player.onLedge) {
				gs->player.onLedge = 0;
				gs->player.dy = -9;
			}
			break;
		case SDLK_r:
			if (over_ready(gs)) {
				Mix_HaltMusic();
				game_reset(gs); // leak #2 fix: state only, no reload
			}
			break;
		case SDLK_x:
			if (over_ready(gs)) return SCENE_MENU;
			break;
		}
	}
	return SCENE_GAMEPLAY;
}

SceneId gameplay_run(App* app) {
	static GameState gs; // ~470KB: keep it off the stack
	SDL_memset(&gs, 0, sizeof gs);
	gs.renderer = app->renderer;
	gs.difficulty = app->difficulty;
	app_resize(app, WIDTH_WINDOW, HEIGHT_WINDOW);

	SceneId next = SCENE_GAMEPLAY;
	if (game_assets_load(&gs)) {
		game_reset(&gs);
	}
	else {
		app_fatal(app, "Missing game resources (see console)");
		next = SCENE_QUIT;
	}
	Uint64 last = SDL_GetTicks64();
	while (next == SCENE_GAMEPLAY) {
		Uint64 now = SDL_GetTicks64();
		gs.dt = (now - last) / 1000.0f;
		last = now;
		next = handle_input(&gs);
		if (next != SCENE_GAMEPLAY) break;
		collision_detect_floor(&gs);
		for (int i = 0; i < gs.map->counter; i++) {
			collision_detect_map(&gs, &gs.map[i]);
		}
		add_physics(&gs);
		do_render(&gs);
		app_tick(app);
	}
	Mix_HaltChannel(-1);
	Mix_HaltMusic();
	game_assets_free(&gs);
	return next;
}
