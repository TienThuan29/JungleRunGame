#pragma once
#include <SDL.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { int left, right, down, jump; } ParityInput;

#define PARITY_MAX_CELLS 8192
typedef struct {
	float x, y, dx, dy, immortalStartTime, bossX, scrollX;
	int lives, isTakenDamage, isImmortal, onLedge, flip, status;
	int potions, keyObtained, statusState, isOver;
	int cells;
	unsigned char lethal[PARITY_MAX_CELLS], solid[PARITY_MAX_CELLS], taken[PARITY_MAX_CELLS];
	float cellX[PARITY_MAX_CELLS];
} ParitySnap;

int c_init(SDL_Renderer* r, int difficulty);
void c_place(float x, float y);
void c_frame(ParityInput in, float dt);
void c_snap(ParitySnap* s);
int c_segment_count(void);
float c_segment_x(int i);
int c_find_key(float* x, float* y);

int cpp_init(SDL_Renderer* r, int difficulty);
void cpp_place(float x, float y);
void cpp_frame(ParityInput in, float dt);
void cpp_snap(ParitySnap* s);
void cpp_shutdown(void);
#ifdef __cplusplus
}
#endif
