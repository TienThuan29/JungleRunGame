#ifndef COLLISION_HANDLING
#define COLLISION_HANDLING
#include "common_functions.h"
void collision_detect_floor(GameState* gameState);
void collision_detect_map(GameState* gameState, Map* map);
void collision_correction(GameState* gameState, float px, float py, float pw, float ph, float bx, float by, float bw, float bh);
#endif