#include "game/game.h"
#include <math.h>

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

void immortal_events(GameState* gameState) {
	float currentTime = SDL_GetTicks64() / 1000.0f;
	if (currentTime - gameState->player.immortalStartTime >= 1.0f) {
		gameState->player.isImmortal = 0;
	}
}

void load_player_info(GameState* gameState, float* px, float* py, float* pw, float* ph) {
	*pw = WIDTH_PLAYER_IDLE * 2, * ph = HEIGHT_PLAYER_IDLE * 2;
	if (gameState->player.status == 0) {
		*pw = WIDTH_PLAYER_IDLE * 2, * ph = HEIGHT_PLAYER_IDLE * 2;
	}
	else if (gameState->player.status == 1) {
		*pw = WIDTH_PLAYER_IDLE * 2, * ph = HEIGHT_PLAYER_IDLE * 2;
	}
	else if (gameState->player.status == 2 || gameState->player.status == 3 || gameState->player.status == 4) {
		*pw = WIDTH_PLAYER_JUMP * 2, * ph = HEIGHT_PLAYER_JUMP * 2;
	}
	*px = gameState->player.x, * py = gameState->player.y;
}
