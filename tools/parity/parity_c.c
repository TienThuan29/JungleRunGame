/* C reference side: the original src/game/*.c, no assets loaded. */
#include "game/game.h"
#include "parity_api.h"
#include <math.h>
#include <string.h>

static GameState gs;

int c_init(SDL_Renderer* r, int difficulty) {
	memset(&gs, 0, sizeof gs);
	gs.renderer = r;
	gs.difficulty = difficulty;
	game_reset(&gs);
	return 1;
}

void c_place(float x, float y) {
	gs.player.x = x;
	gs.player.y = y;
}

/* Literal copy of the movement part of handle_input() in src/scenes/gameplay.c, keys from `in`. */
static void c_input(ParityInput in) {
	if (in.left) {
		gs.player.status = 1;
		gs.player.flip = 1;
		gs.player.dx -= 0.3f;
		if (gs.player.dx < -2.5f) gs.player.dx = -2.5f;
	}
	else if (in.right) {
		gs.player.status = 1;
		gs.player.flip = 0;
		gs.player.dx += 0.3f;
		if (gs.player.dx > 2.5f) gs.player.dx = 2.5f;
	}
	else if (in.down) {
		gs.player.dy += 0.3f;
	}
	else {
		gs.player.status = 0;
		gs.player.flip = 0;
		gs.player.dx *= 0.8f;
		if (fabsf(gs.player.dx) < 0.1f) gs.player.dx = 0;
	}
	if (in.jump && gs.player.onLedge) {
		gs.player.onLedge = 0;
		gs.player.dy = -9;
	}
}

void c_frame(ParityInput in, float dt) { /* same order as gameplay_run() */
	gs.dt = dt;
	c_input(in);
	collision_detect_floor(&gs);
	for (int i = 0; i < gs.map->counter; i++) collision_detect_map(&gs, &gs.map[i]);
	add_physics(&gs);
	do_render(&gs);
}

void c_snap(ParitySnap* s) {
	s->x = gs.player.x;
	s->y = gs.player.y;
	s->dx = gs.player.dx;
	s->dy = gs.player.dy;
	s->immortalStartTime = gs.player.immortalStartTime;
	s->bossX = gs.gigaGolem.pos.x;
	s->scrollX = gs.scrollX;
	s->lives = gs.player.lives;
	s->isTakenDamage = gs.player.isTakenDamage;
	s->isImmortal = gs.player.isImmortal;
	s->onLedge = gs.player.onLedge;
	s->flip = gs.player.flip;
	s->status = gs.player.status;
	s->potions = gs.health_potion_counter;
	s->keyObtained = gs.isKeyObtained;
	s->statusState = gs.statusState;
	s->isOver = gs.isOver;
	s->cells = 0;
	for (int m = 0; m < gs.map->counter; m++)
		for (int row = 0; row < MAP_HEIGHT; row++)
			for (int col = 0; col < MAP_WIDTH; col++) {
				if (gs.map[m].pos[row][col] == 0 || s->cells == PARITY_MAX_CELLS) continue;
				const Ledge* l = &gs.map[m].ledges[row][col];
				s->lethal[s->cells] = (unsigned char)l->isLethal;
				s->solid[s->cells] = (unsigned char)l->isBlocked;
				s->taken[s->cells] = (unsigned char)l->isObtained;
				s->cellX[s->cells] = l->x;
				s->cells++;
			}
}

int c_segment_count(void) { return gs.map->counter; }
float c_segment_x(int i) { return gs.map[i].ledges[0][0].x; }

int c_find_key(float* x, float* y) {
	for (int m = 0; m < gs.map->counter; m++)
		for (int row = 0; row < MAP_HEIGHT; row++)
			for (int col = 0; col < MAP_WIDTH; col++)
				if (gs.map[m].pos[row][col] == 21) {
					*x = gs.map[m].ledges[row][col].x;
					*y = gs.map[m].ledges[row][col].y;
					return 1;
				}
	return 0;
}
