#include "common_functions.h"
#include "load.h"
#include "render.h"
#include "collision_handling.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "map.h"

void gamePlay(GameState* gameState, SDL_Renderer* renderer, SDL_Window* window) {
	// FPS
	Uint64 lastFrameTime = 0;
	// Load game
	gameState->renderer = renderer;
	load_game(gameState);
	// Enter program loop
	int done = 0;
	while (!done) {
		Uint64 currentFrameTime = SDL_GetTicks64();
		Uint64 elapsedTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;
		done = processEvents(window, gameState);
		if (!done) {
			// detect ground floor (platform[2])
			collision_detect_floor(gameState);

			// detect map
			for (int i = 0; i < gameState->map->counter; i++) {
				collision_detect_map(gameState, &gameState->map[i]);
			}

			add_physics(gameState);

			do_render(gameState, &done);

			// Calculate and print framerate
			gameState->dt = elapsedTime / 1000.0f;
		}
	}
	// Close and destroy the window
	clean(gameState, window);
}

int processEvents(SDL_Window* window, GameState* gameState) {
	// Walking
	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) {
		gameState->player.status = 1;
		gameState->player.flip = 1;
		gameState->player.dx -= 0.3f;
		if (gameState->player.dx < -2.5) {
			gameState->player.dx = -2.5;
		}
	}
	else if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) {
		gameState->player.status = 1;
		gameState->player.flip = 0;
		gameState->player.dx += 0.3f;
		if (gameState->player.dx > 2.5) {
			gameState->player.dx = 2.5;
		}
	}
	else if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S]) {
		gameState->player.dy += 0.3f;
	}
	else {
		gameState->player.status = 0;
		gameState->player.flip = 0;
		gameState->player.dx *= 0.8f;
		if (fabsf(gameState->player.dx) < 0.1f) {
			gameState->player.dx = 0;
		}
	}
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		// Exit program
		switch (event.type) {
		case SDL_WINDOWEVENT_CLOSE:
			if (window) {
				Mix_HaltMusic();
				SDL_DestroyWindow(window);
				window = NULL;
				return 1;
			}
			break;
		case SDL_QUIT:
			Mix_HaltMusic();
			return 1;
		}
		// Interact with keyboard
		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
			case SDLK_SPACE:
			case SDLK_UP:
			case SDLK_w:
				if (gameState->player.onLedge) {
					gameState->player.onLedge = 0;
					gameState->player.dy = -9;
				}
				break;
			}
		}
	}
}

void add_physics(GameState* gameState) {
	Object* player = &gameState->player;
	if (player->x >= END_OF_GAME_POSITION && !gameState->isOver) {
		gameState->isOver = 1;
		Mix_HaltMusic();
		gameState->overTiming = SDL_GetTicks64() / 1000.0f;
		if (gameState->health_potion_counter < REQUIRED_HEALTH_POTION) {
			gameState->statusState = STATUS_STATE_TRUE_END; // true ending
		}
		else if (gameState->health_potion_counter == REQUIRED_HEALTH_POTION) {
			gameState->statusState = STATUS_STATE_GOOD_END; // good ending
		}
	}
	else if ((touchBoss(gameState) || player->lives == 0 || player->y>HEIGHT_WINDOW+250) && !gameState->isOver) {	// gameover <-> bad ending
		gameState->isOver = 1;
		Mix_HaltChannel(-1);
		Mix_HaltChannel(1);
		Mix_HaltMusic();
		gameState->overTiming = SDL_GetTicks64() / 1000.0f;
		gameState->statusState = STATUS_STATE_GAMEOVER;
	}
	else {
		if (player->isImmortal) {
			immortal_events(gameState);
		}
		player->x += player->dx;
		player->y += player->dy;
		player->dy += GRAVITY;
		if (!player->onLedge) {
			if (player->dy < 0) {
				player->status = 2;
				if (!Mix_Playing(1)) {
					Mix_PlayChannel(1, gameState->soundEffects.jump, 0);
				}
			}
			else if (player->dy == 0) {
				player->status = 3;
			}
			else {
				player->status = 4;
			}
		}
		// scrolling
		gameState->scrollX = -player->x + 350;
		if (gameState->scrollX > 0) {
			gameState->scrollX = 0;
		}
		float currentTime = SDL_GetTicks64() / 1000.0f;
		if (currentTime - gameState->bossStart >= 0.5f) {
			updateBoss(gameState, gameState->dt);
		}
	}
}

int animation_smoothness(int frame, const int frames) {
	size_t animation_speed = SDL_GetTicks64() / frames;
	int i = animation_speed % frame;
	return i;
}

void onTrapHit(GameState* gameState, SDL_Texture* texture, int x, int y) {
	SDL_SetRenderTarget(gameState->renderer, texture);
	// Set the color modulation to red
	SDL_SetTextureColorMod(texture, 255, 0, 0);
	// Render the texture to the renderer at the specified location
	SDL_Rect dstRect = { x, y, 0, 0 };
	SDL_QueryTexture(texture, NULL, NULL, &dstRect.w, &dstRect.h);
	dstRect.w *= 2;
	dstRect.h *= 2;
	SDL_RenderCopyEx(gameState->renderer, texture, NULL, &dstRect, 0, NULL, gameState->player.flip);
	SDL_RenderPresent(gameState->renderer);
	// Reset the color modulation to white
	SDL_SetTextureColorMod(texture, 255, 255, 255);
	SDL_RenderCopyEx(gameState->renderer, texture, NULL, &dstRect, 0, NULL, gameState->player.flip);
}

// bot golem detect player
void detectPlayer(GameState* gameState, Map* map, int row, int col, float dt, float px, float py, float pw, float ph) {
	Ledge* golem = &map->ledges[row][col];
	float bw = WIDTH_PLATFORM_6, bh = HEIGHT_PLATFORM_6, bx = golem->x, by = golem->y;
	if (px + pw >= bx - SMOL_GOLEM_DETECT_RANGE && px <= bx + bw + SMOL_GOLEM_DETECT_RANGE && fabs(by-py)<=2 * HEIGHT_PLATFORM_6) { // detect player
		if (px < bx + (bw / 2)) { // player in left , golem turn left
			golem->smolGolem.d.x += -SMOL_GOLEM_SPEED;
			golem->smolGolem.flip = 1;
		}
		else if (px > bx + (bw / 2)) { // player in right, golem turn right
			golem->smolGolem.d.x += SMOL_GOLEM_SPEED;
			golem->smolGolem.flip = 0;
		}
	}
	else golem->smolGolem.d.x = 0.f;
}

void updateGolem(Map* map, int row, int col, float dt) {
	Ledge* golem = &map->ledges[row][col];
	if (golem->smolGolem.d.x > 120) {
		golem->smolGolem.d.x = 120;
	} 
	if (golem->smolGolem.d.x < -120) {
		golem->smolGolem.d.x = -120;
	}
	golem->x += golem->smolGolem.d.x * dt;
}

void updateBoss(GameState* gameState, float dt) {
	Golem* boss = &gameState->gigaGolem;
	float camX = -gameState->scrollX;
	if (camX - boss->pos.x > 0 && gameState->player.dx > 0) { // neu player chay, toc do golem = toc do cua player de bat kip player
		boss->d.x += gameState->player.dx;
	}
	else {
		boss->d.x = GIGA_GOLEM_SPEED;
	} // neu phayer dung lai thi golem se di voi toc do cua golem
	boss->pos.x += boss->d.x * dt;
}

int touchBoss(GameState* gameState) { // return true if touched, false if not touched
	Golem boss = gameState->gigaGolem;
	// init info
	float px, py, pw, ph;
	load_player_info(gameState, &px, &py, &pw, &ph);
	float bx = boss.pos.x, bw = WIDTH_GIGA_GOLEM;
	if (px <= (bx + 0.75*bw)) {
		return 1;
	}
	return 0;
}

void Over(GameState* gameState, const char str[], SDL_Color fg, int* done) {
	Mix_HaltChannel(-1);
	Mix_HaltChannel(1);
	if (gameState->statusState == STATUS_STATE_GAMEOVER) {
		if (!Mix_PlayingMusic()) {
			Mix_PlayMusic(gameState->soundTracks.ending[STATUS_STATE_GAMEOVER], -1);
		}
	}
	else if (gameState->statusState == STATUS_STATE_TRUE_END) {
		if (!Mix_PlayingMusic()) {
			Mix_PlayMusic(gameState->soundTracks.ending[STATUS_STATE_TRUE_END], -1);
		}
	}
	else if (gameState->statusState == STATUS_STATE_GOOD_END) {
		if (!Mix_PlayingMusic()) {
			Mix_PlayMusic(gameState->soundTracks.ending[STATUS_STATE_GOOD_END], -1);
		}
	}
	// background 
	for (int i = 0; i < 5; i++) {
		SDL_RenderCopy(gameState->renderer, gameState->background[i].layer[0], NULL, NULL);
	}
	gameState->font = NULL;
	gameState->font = TTF_OpenFont("Resource\\Fonts\\Kanit-Light.ttf", 108);
	SDL_Rect textRect;
	SDL_Surface* textSurface = TTF_RenderText_Solid(gameState->font, str, fg);
	SDL_Texture* textTexture = SDL_CreateTextureFromSurface(gameState->renderer, textSurface);
	SDL_FreeSurface(textSurface);
	SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
	textRect.x = (WIDTH_WINDOW - textRect.w) / 2;
	textRect.y = (HEIGHT_WINDOW - textRect.h) / 2;
	SDL_RenderCopy(gameState->renderer, textTexture, NULL, &textRect);
	// Display text: press R to retry, press X to return menu
	float currentTime = SDL_GetTicks64() / 1000.0f;
	if (currentTime - gameState->overTiming >= 2.0f) {
		TTF_Font* informativeText = TTF_OpenFont("Resource\\Fonts\\crazy-pixel.ttf", 48);
		SDL_Rect informativeRect;
		textSurface = TTF_RenderText_Solid(informativeText, "Press R to retry, X to return menu", (SDL_Color) { 255, 255, 73, 255 });
		SDL_Texture* informativeTexture = SDL_CreateTextureFromSurface(gameState->renderer, textSurface);
		SDL_FreeSurface(textSurface);
		SDL_QueryTexture(informativeTexture, NULL, NULL, &informativeRect.w, &informativeRect.h);
		informativeRect.x = 0;
		informativeRect.y = -15;
		SDL_RenderCopy(gameState->renderer, informativeTexture, NULL, &informativeRect);
		// handle events
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			// Interact with keyboard
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_r: // retry
					load_game(gameState);
					Mix_HaltMusic();
					break;
				case SDLK_x: // return menu
					*done = 1;
					Mix_HaltMusic();
					break;
				}
			}
		}
	}
}

void immortal_events(GameState* gameState) {
	float currentTime = SDL_GetTicks64() / 1000.0f;
	if (currentTime - gameState->player.immortalStartTime >= 1.0f) {
		gameState->player.isImmortal = 0;
	}
}

void clean(GameState* gameState, SDL_Window* window) {
	SDL_DestroyTexture(gameState->idle_anim);
	SDL_DestroyTexture(gameState->run_anim);
	SDL_DestroyTexture(gameState->jump_anim);
	SDL_DestroyTexture(gameState->platform);
	for (int i = 0; i < 5; i++) {
		SDL_DestroyTexture(gameState->background[i].layer);
	}
	SDL_DestroyTexture(gameState->golem);
	SDL_DestroyTexture(gameState->fire_trap);
	if (gameState->label_lives != NULL) {
		SDL_DestroyTexture(gameState->label_lives);
	}
	if (gameState->label_health_potion != NULL) {
		SDL_DestroyTexture(gameState->label_health_potion);
	}
	TTF_CloseFont(gameState->font);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(gameState->renderer);
	Mix_FreeChunk(gameState->soundEffects.electricHurt);
	Mix_FreeChunk(gameState->soundEffects.getDamaged);
	Mix_FreeChunk(gameState->soundEffects.ItemPickUp);
	Mix_FreeChunk(gameState->soundEffects.jump);
	Mix_FreeChunk(gameState->soundEffects.landing);
	Mix_FreeChunk(gameState->soundEffects.spike);
	Mix_FreeMusic(gameState->soundTracks.inGame[0]);
}