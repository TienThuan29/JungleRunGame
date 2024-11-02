#include "render.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "map.h"

void do_render(GameState* gameState, int* done) {
	SDL_Renderer* renderer = gameState->renderer;
	SDL_RenderClear(renderer);
	if (gameState->statusState == STATUS_STATE_GAMEOVER) {
		Over(gameState, "Bad End", (SDL_Color) { 210, 8, 45, 255 }, done);
	}
	else if (gameState->statusState == STATUS_STATE_TRUE_END) {
		Over(gameState, "True End", (SDL_Color) { 200, 112, 126, 255 }, done);
	}
	else if (gameState->statusState == STATUS_STATE_GOOD_END) {
		Over(gameState, "Good End", (SDL_Color) { 96, 106, 116, 255 }, done);
	}
	else if (gameState->statusState == STATUS_STATE_GAME) {
		// draw background
		drawBackground(gameState);
		// draw Map
		for (int i = 0; i < gameState->map->counter; i++) {
			if (gameState->isKeyObtained && !gameState->flag) {
				draw_key_map(gameState, 23);
			}
			// draw map
			drawMap(gameState, &gameState->map[i]);
		}
		// draw ledges
		drawLedges(gameState);
		if (gameState->player.status == 0) { // idle 
			int i = animation_smoothness(12, CHARACTER_FRAMES);
			render_character_animation(gameState, gameState->idle_anim, WIDTH_PLAYER_IDLE, HEIGHT_PLAYER_IDLE, i);
		}
		else if (gameState->player.status == 1) { // run
			int i = animation_smoothness(8, CHARACTER_FRAMES);
			render_character_animation(gameState, gameState->run_anim, WIDTH_PLAYER_RUN, HEIGHT_PLAYER_RUN, i);
		}
		else if (gameState->player.status == 2) { // jump
			render_character_animation(gameState, gameState->jump_anim, WIDTH_PLAYER_JUMP, HEIGHT_PLAYER_JUMP, 0);
		}
		else if (gameState->player.status == 3) { // mid air
			render_character_animation(gameState, gameState->jump_anim, WIDTH_PLAYER_JUMP, HEIGHT_PLAYER_JUMP, 1);
		}
		else if (gameState->player.status == 4) { // falling
			int i = animation_smoothness(2, CHARACTER_FRAMES) + 2;
			render_character_animation(gameState, gameState->jump_anim, WIDTH_PLAYER_JUMP, HEIGHT_PLAYER_JUMP, i);
		}
		// draw giga golem
		render_giga_golem(gameState, gameState->golem);
		// draw status
		render_status(gameState);
		// Music
		if (!Mix_PlayingMusic()) {
			Mix_PlayMusic(gameState->soundTracks.inGame[0], -1);
		}
	}
	SDL_RenderPresent(renderer);
}

void render_character_animation(GameState* gameState, SDL_Texture* texture[], const int width, const int height, int frame) {
	if (gameState->player.isTakenDamage && gameState->player.isImmortal) {
		onTrapHit(gameState, texture[frame], gameState->scrollX + gameState->player.x, gameState->player.y);
	}
	else {
		SDL_Rect dstRect = { gameState->scrollX + gameState->player.x, gameState->player.y, width * 2, height * 2 };
		SDL_RenderCopyEx(gameState->renderer, texture[frame], NULL, &dstRect, 0, NULL, gameState->player.flip);
	}
}

void render_giga_golem(GameState* gameState, SDL_Texture* texture[]) {
	int smoothness = animation_smoothness(10, GIGA_GOLEM_FRAMES);
	SDL_Rect srcRect = { 24 + smoothness * 100, 624, 50, 50 };
	Golem boss = gameState->gigaGolem;
	SDL_Rect dstRect = { gameState->scrollX + boss.pos.x, boss.pos.y, WIDTH_GIGA_GOLEM, HEIGHT_GIGA_GOLEM };
	SDL_RenderCopyEx(gameState->renderer, texture[0], &srcRect, &dstRect, 0, NULL, 0);
}

void render_status(GameState* gameState) {
	// load label hearts
	gameState->font = NULL;
	gameState->font = TTF_OpenFont("Resource\\Fonts\\crazy-pixel.ttf", 48);
	SDL_Rect textRect1;
	char str1[128] = "";
	sprintf_s(str1, 128, "%hi", gameState->player.lives);
	SDL_Surface* textSurface = TTF_RenderText_Solid(gameState->font, str1, (SDL_Color) { 255, 255, 255, 255 });
	gameState->label_lives = SDL_CreateTextureFromSurface(gameState->renderer, textSurface);
	SDL_FreeSurface(textSurface);
	SDL_QueryTexture(gameState->label_lives, NULL, NULL, &textRect1.w, &textRect1.h);
	textRect1.x = 5;
	textRect1.y = -15;
	// load label health potion
	SDL_Rect textRect2;
	char str2[128] = "";
	sprintf_s(str1, 128, "%d", gameState->health_potion_counter);
	textSurface = TTF_RenderText_Solid(gameState->font, str1, (SDL_Color) { 255, 255, 255, 255 });
	gameState->label_health_potion = SDL_CreateTextureFromSurface(gameState->renderer, textSurface);
	SDL_FreeSurface(textSurface);
	SDL_QueryTexture(gameState->label_health_potion, NULL, NULL, &textRect2.w, &textRect2.h);
	textRect2.x = 200;
	textRect2.y = -15;
	// load hearts texture
	int smoothness1 = animation_smoothness(6, FIRST_AID_KIT_FRAMES);
	SDL_Rect srcRect1 = { smoothness1 * 32, 1 * 32, 32, 32 };
	// load potion counter texture
	int smoothness2 = animation_smoothness(8, FIRST_AID_KIT_FRAMES);
	SDL_Rect srcRect2 = (SDL_Rect){ smoothness2 * 32, 9 * 32, 32, 32 };
	SDL_RenderCopy(gameState->renderer, gameState->label_lives, NULL, &textRect1);
	SDL_RenderCopy(gameState->renderer, gameState->items[0], &srcRect1, &(SDL_Rect) { textRect1.x + textRect1.w, 0, 48, 48 });
	SDL_RenderCopy(gameState->renderer, gameState->label_health_potion, NULL, &textRect2);
	SDL_RenderCopy(gameState->renderer, gameState->items[0], &srcRect2, &(SDL_Rect) { textRect2.x + textRect2.w, 0, 48, 48 });
}