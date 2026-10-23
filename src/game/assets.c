#include "game/game.h"
#include "core/res.h"

#define COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))
#define TRAPS "Resource\\Foozle_2DTR0001_Pixel_Trap_Pack\\"

static int all_set(SDL_Texture* const t[], int n) {
	for (int i = 0; i < n; i++) {
		if (!t[i]) return 0;
	}
	return 1;
}

int game_assets_load(GameState* gs) {
	SDL_Renderer* r = gs->renderer;
	static const int scrollSpeed[5] = { PLX_BACKGROUND_1, PLX_BACKGROUND_2, PLX_BACKGROUND_3, PLX_BACKGROUND_4, PLX_BACKGROUND_5 };

	res_texture_seq(r, gs->idle_anim, COUNT(gs->idle_anim), "Resource\\Character\\Idle\\Idle_%d.png", 1);
	res_texture_seq(r, gs->run_anim, COUNT(gs->run_anim), "Resource\\Character\\Run\\Run_%d.png", 1);
	res_texture_seq(r, gs->jump_anim, COUNT(gs->jump_anim), "Resource\\Character\\Jump\\Jump_%d.png", 1);
	gs->golem[0] = res_texture(r, "Resource\\Mecha-stone Golem 0.1\\PNG sheet\\Character_sheet.png", 1);
	for (int i = 0; i < 5; i++) {
		char path[128];
		SDL_snprintf(path, sizeof path, "Resource\\Jungle Asset Pack\\parallax background\\plx-%d.png", i + 1);
		gs->background[i].scrollSpeed = scrollSpeed[i];
		for (int j = 0; j < 3; j++) {
			gs->background[i].layer[j] = res_texture(r, path, 0);
		}
	}
	res_texture_seq(r, gs->platform, 5, "Resource\\Some Bullshit Platform\\Platform %d.png", 1);
	gs->platform[5] = res_texture(r, "Resource\\Jungle Asset Pack\\jungle tileset\\jungle tileset.png", 0);
	res_texture_seq(r, gs->fire_trap, COUNT(gs->fire_trap), TRAPS "Fire Trap\\PNGs\\Fire Trap - Level %d.png", 1);
	res_texture_seq(r, gs->ceiling_trap, COUNT(gs->ceiling_trap), TRAPS "Ceiling Trap\\PNGs\\Ceiling Trap - Level %d.png", 1);
	res_texture_seq(r, gs->saw_trap, COUNT(gs->saw_trap), TRAPS "Saw Trap\\PNGs\\Saw Trap - Level %d.png", 1);
	res_texture_seq(r, gs->lightning_trap, COUNT(gs->lightning_trap), TRAPS "Lightning Trap\\PNGs\\Lightning Trap - Level %d.png", 1);
	gs->items[0] = res_texture(r, "Resource\\platformer items\\animated_items.png", 1);
	gs->sparkling_star[0] = res_texture(r, "Resource\\sparkle\\sparkle.png", 1);

	gs->hud_font = res_font("Resource\\Fonts\\crazy-pixel.ttf", 48);
	gs->over_font = res_font("Resource\\Fonts\\Kanit-Light.ttf", 108);

	SoundEffect* se = &gs->soundEffects;
	se->getDamaged = res_wav("Resource\\Sound\\Sound Effects\\DamageSound.wav", 64);
	se->electricHurt = res_wav("Resource\\Sound\\Sound Effects\\ElectricHurtSound.wav", 64);
	se->ItemPickUp = res_wav("Resource\\Sound\\Sound Effects\\ItemPickupSound.wav", 96);
	se->jump = res_wav("Resource\\Sound\\Sound Effects\\jump.wav", 16);
	se->landing = res_wav("Resource\\Sound\\Sound Effects\\LandingSound.wav", 16);
	se->spike = res_wav("Resource\\Sound\\Sound Effects\\SpikeSound.wav", 32);

	Music* m = &gs->soundTracks;
	m->inGame[0] = res_mus("Resource\\Sound\\Soundtrack\\In Game 3.mp3");
	m->ending[STATUS_STATE_GAMEOVER] = res_mus("Resource\\Sound\\Soundtrack\\Bad Ending.mp3");
	m->ending[STATUS_STATE_TRUE_END] = res_mus("Resource\\Sound\\Soundtrack\\True Ending.mp3");
	m->ending[STATUS_STATE_GOOD_END] = res_mus("Resource\\Sound\\Soundtrack\\Good Ending.mp3");
	Mix_VolumeMusic(32);

	// sounds are optional (game runs silent, as before); images and fonts are not
	int ok = gs->hud_font && gs->over_font
		&& all_set(gs->idle_anim, COUNT(gs->idle_anim)) && all_set(gs->run_anim, COUNT(gs->run_anim))
		&& all_set(gs->jump_anim, COUNT(gs->jump_anim)) && all_set(gs->golem, COUNT(gs->golem))
		&& all_set(gs->platform, COUNT(gs->platform)) && all_set(gs->fire_trap, COUNT(gs->fire_trap))
		&& all_set(gs->ceiling_trap, COUNT(gs->ceiling_trap)) && all_set(gs->saw_trap, COUNT(gs->saw_trap))
		&& all_set(gs->lightning_trap, COUNT(gs->lightning_trap)) && all_set(gs->items, COUNT(gs->items))
		&& all_set(gs->sparkling_star, COUNT(gs->sparkling_star));
	for (int i = 0; i < 5 && ok; i++) {
		ok = all_set(gs->background[i].layer, 3);
	}
	return ok;
}

void game_assets_free(GameState* gs) {
	res_destroy_seq(gs->idle_anim, COUNT(gs->idle_anim));
	res_destroy_seq(gs->run_anim, COUNT(gs->run_anim));
	res_destroy_seq(gs->jump_anim, COUNT(gs->jump_anim));
	res_destroy_seq(gs->golem, COUNT(gs->golem));
	for (int i = 0; i < 5; i++) {
		res_destroy_seq(gs->background[i].layer, 3);
	}
	res_destroy_seq(gs->platform, COUNT(gs->platform));
	res_destroy_seq(gs->fire_trap, COUNT(gs->fire_trap));
	res_destroy_seq(gs->ceiling_trap, COUNT(gs->ceiling_trap));
	res_destroy_seq(gs->saw_trap, COUNT(gs->saw_trap));
	res_destroy_seq(gs->lightning_trap, COUNT(gs->lightning_trap));
	res_destroy_seq(gs->items, COUNT(gs->items));
	res_destroy_seq(gs->sparkling_star, COUNT(gs->sparkling_star));
	res_destroy_texture(&gs->label_lives);
	res_destroy_texture(&gs->label_health_potion);
	res_destroy_texture(&gs->over_title);
	res_destroy_texture(&gs->over_hint);
	res_close_font(&gs->hud_font);
	res_close_font(&gs->over_font);
	SoundEffect* se = &gs->soundEffects;
	res_free_wav(&se->getDamaged);
	res_free_wav(&se->electricHurt);
	res_free_wav(&se->ItemPickUp);
	res_free_wav(&se->jump);
	res_free_wav(&se->landing);
	res_free_wav(&se->spike);
	res_free_mus(&gs->soundTracks.inGame[0]);
	for (int i = 0; i < 3; i++) {
		res_free_mus(&gs->soundTracks.ending[i]);
	}
}

void game_reset(GameState* gs) {
	gs->mode = GAMEMODE_SINGLEPLAYER;
	gs->dt = 0;
	gs->overTiming = 0;
	gs->statusState = STATUS_STATE_GAME;
	gs->isOver = 0;
	gs->player = (Object){ .x = 500, .y = 220 };
	if (gs->difficulty == DIFFICULTY_MEDIUM) gs->player.lives = MAX_HEALTH_MEDIUM;
	else if (gs->difficulty == DIFFICULTY_HARD) gs->player.lives = MAX_HEALTH_HARD;
	else gs->player.lives = MAX_HEALTH_EASY;
	gs->health_potion_counter = 0;
	gs->isKeyObtained = 0;
	gs->flag = 0;
	gs->bossStart = SDL_GetTicks64() / 1000.0f;
	gs->gigaGolem = (Golem){ .pos = { 0, (HEIGHT_WINDOW - HEIGHT_GIGA_GOLEM) / 2.0f } };
	gs->scrollX = 0;
	for (int i = 0; i < 5; i++) {
		gs->background[i].defaultX = 0;
		gs->background[i].previousX = -WIDTH_WINDOW;
		gs->background[i].nextX = WIDTH_WINDOW;
	}
	res_destroy_texture(&gs->over_title); // next ending may have a different title
	res_destroy_texture(&gs->over_hint);
	gs->map->counter = 0;
	drawMap_SingleplayerMode(gs);
}
