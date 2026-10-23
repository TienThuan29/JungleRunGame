#include "game/game.h"
#include "core/res.h"

static const SDL_Color WHITE = { 255, 255, 255, 255 };

// Blink red while immortal after a hit.
static void draw_player_hit(GameState* gs, SDL_Texture* t, int x, int y) {
	SDL_Rect dst = { x, y, 0, 0 };
	SDL_QueryTexture(t, NULL, NULL, &dst.w, &dst.h);
	dst.w *= 2;
	dst.h *= 2;
	int red = (int)((SDL_GetTicks64() / 100) % 2);
	if (red) SDL_SetTextureColorMod(t, 255, 0, 0);
	SDL_RenderCopyEx(gs->renderer, t, NULL, &dst, 0, NULL, gs->player.flip);
	if (red) SDL_SetTextureColorMod(t, 255, 255, 255);
}

static void render_character_animation(GameState* gs, SDL_Texture* texture[], const int width, const int height, int frame) {
	int x = (int)(gs->scrollX + gs->player.x), y = (int)gs->player.y;
	if (gs->player.isTakenDamage && gs->player.isImmortal) {
		draw_player_hit(gs, texture[frame], x, y);
	}
	else {
		SDL_Rect dstRect = { x, y, width * 2, height * 2 };
		SDL_RenderCopyEx(gs->renderer, texture[frame], NULL, &dstRect, 0, NULL, gs->player.flip);
	}
}

static void render_giga_golem(GameState* gs, SDL_Texture* texture[]) {
	int smoothness = animation_smoothness(10, GIGA_GOLEM_FRAMES);
	SDL_Rect srcRect = { 24 + smoothness * 100, 624, 50, 50 };
	Golem boss = gs->gigaGolem;
	SDL_Rect dstRect = { (int)(gs->scrollX + boss.pos.x), (int)boss.pos.y, WIDTH_GIGA_GOLEM, HEIGHT_GIGA_GOLEM };
	SDL_RenderCopyEx(gs->renderer, texture[0], &srcRect, &dstRect, 0, NULL, 0);
}

// Leak #1 fix: text texture is rebuilt only when the number changes.
static void hud_label(GameState* gs, SDL_Texture** tex, int* cached, int value) {
	if (*tex && *cached == value) return;
	res_destroy_texture(tex);
	char s[16];
	SDL_snprintf(s, sizeof s, "%d", value);
	*tex = res_text(gs->renderer, gs->hud_font, s, WHITE);
	*cached = value;
}

static void render_status(GameState* gs) {
	hud_label(gs, &gs->label_lives, &gs->cached_lives, gs->player.lives);
	hud_label(gs, &gs->label_health_potion, &gs->cached_potions, gs->health_potion_counter);
	SDL_Rect textRect1 = { 5, -15, 0, 0 };
	SDL_QueryTexture(gs->label_lives, NULL, NULL, &textRect1.w, &textRect1.h);
	SDL_Rect textRect2 = { 200, -15, 0, 0 };
	SDL_QueryTexture(gs->label_health_potion, NULL, NULL, &textRect2.w, &textRect2.h);
	int smoothness1 = animation_smoothness(6, FIRST_AID_KIT_FRAMES);
	SDL_Rect srcRect1 = { smoothness1 * 32, 1 * 32, 32, 32 };
	int smoothness2 = animation_smoothness(8, FIRST_AID_KIT_FRAMES);
	SDL_Rect srcRect2 = { smoothness2 * 32, 9 * 32, 32, 32 };
	SDL_RenderCopy(gs->renderer, gs->label_lives, NULL, &textRect1);
	SDL_RenderCopy(gs->renderer, gs->items[0], &srcRect1, &(SDL_Rect){ textRect1.x + textRect1.w, 0, 48, 48 });
	SDL_RenderCopy(gs->renderer, gs->label_health_potion, NULL, &textRect2);
	SDL_RenderCopy(gs->renderer, gs->items[0], &srcRect2, &(SDL_Rect){ textRect2.x + textRect2.w, 0, 48, 48 });
}

// Leak #1b fix: title/hint created once per ending (game_reset destroys them).
static void render_over(GameState* gs, const char* str, SDL_Color fg) {
	Mix_HaltChannel(-1);
	if (!Mix_PlayingMusic()) {
		Mix_PlayMusic(gs->soundTracks.ending[gs->statusState], -1); // statusState 0..2 == ending index
	}
	for (int i = 0; i < 5; i++) {
		SDL_RenderCopy(gs->renderer, gs->background[i].layer[0], NULL, NULL);
	}
	if (!gs->over_title) {
		gs->over_title = res_text(gs->renderer, gs->over_font, str, fg);
	}
	SDL_Rect t = { 0 };
	SDL_QueryTexture(gs->over_title, NULL, NULL, &t.w, &t.h);
	t.x = (WIDTH_WINDOW - t.w) / 2;
	t.y = (HEIGHT_WINDOW - t.h) / 2;
	SDL_RenderCopy(gs->renderer, gs->over_title, NULL, &t);
	if (SDL_GetTicks64() / 1000.0f - gs->overTiming >= 2.0f) {
		if (!gs->over_hint) {
			gs->over_hint = res_text(gs->renderer, gs->hud_font, "Press R to retry, X to return menu", (SDL_Color){ 255, 255, 73, 255 });
		}
		SDL_Rect h = { 0, -15, 0, 0 };
		SDL_QueryTexture(gs->over_hint, NULL, NULL, &h.w, &h.h);
		SDL_RenderCopy(gs->renderer, gs->over_hint, NULL, &h);
	}
}

void do_render(GameState* gs) {
	SDL_Renderer* renderer = gs->renderer;
	SDL_RenderClear(renderer);
	if (gs->statusState == STATUS_STATE_GAMEOVER) {
		render_over(gs, "Bad End", (SDL_Color){ 210, 8, 45, 255 });
	}
	else if (gs->statusState == STATUS_STATE_TRUE_END) {
		render_over(gs, "True End", (SDL_Color){ 200, 112, 126, 255 });
	}
	else if (gs->statusState == STATUS_STATE_GOOD_END) {
		render_over(gs, "Good End", (SDL_Color){ 96, 106, 116, 255 });
	}
	else if (gs->statusState == STATUS_STATE_GAME) {
		drawBackground(gs);
		for (int i = 0; i < gs->map->counter; i++) {
			if (gs->isKeyObtained && !gs->flag) {
				draw_key_map(gs, 23);
			}
			drawMap(gs, &gs->map[i]);
		}
		drawLedges(gs);
		if (gs->player.status == 0) { // idle
			render_character_animation(gs, gs->idle_anim, WIDTH_PLAYER_IDLE, HEIGHT_PLAYER_IDLE, animation_smoothness(12, CHARACTER_FRAMES));
		}
		else if (gs->player.status == 1) { // run
			render_character_animation(gs, gs->run_anim, WIDTH_PLAYER_RUN, HEIGHT_PLAYER_RUN, animation_smoothness(8, CHARACTER_FRAMES));
		}
		else if (gs->player.status == 2) { // jump
			render_character_animation(gs, gs->jump_anim, WIDTH_PLAYER_JUMP, HEIGHT_PLAYER_JUMP, 0);
		}
		else if (gs->player.status == 3) { // mid air
			render_character_animation(gs, gs->jump_anim, WIDTH_PLAYER_JUMP, HEIGHT_PLAYER_JUMP, 1);
		}
		else if (gs->player.status == 4) { // falling
			render_character_animation(gs, gs->jump_anim, WIDTH_PLAYER_JUMP, HEIGHT_PLAYER_JUMP, animation_smoothness(2, CHARACTER_FRAMES) + 2);
		}
		render_giga_golem(gs, gs->golem);
		render_status(gs);
		if (!Mix_PlayingMusic()) {
			Mix_PlayMusic(gs->soundTracks.inGame[0], -1);
		}
	}
	SDL_RenderPresent(renderer);
}
