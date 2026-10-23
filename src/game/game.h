#ifndef GAME_H
#define GAME_H
#include "game/gamestate.h"

// levels.c
extern int lv0[MAP_HEIGHT][MAP_WIDTH], lv1[MAP_HEIGHT][MAP_WIDTH], lv2[MAP_HEIGHT][MAP_WIDTH], lv3[MAP_HEIGHT][MAP_WIDTH],
	lv4[MAP_HEIGHT][MAP_WIDTH], lv5[MAP_HEIGHT][MAP_WIDTH], lv6[MAP_HEIGHT][MAP_WIDTH], lv7[MAP_HEIGHT][MAP_WIDTH],
	lv8[MAP_HEIGHT][MAP_WIDTH], lv9[MAP_HEIGHT][MAP_WIDTH], lv10[MAP_HEIGHT][MAP_WIDTH], lv11[MAP_HEIGHT][MAP_WIDTH],
	lv12[MAP_HEIGHT][MAP_WIDTH], lv13[MAP_HEIGHT][MAP_WIDTH], lv14[MAP_HEIGHT][MAP_WIDTH], lv15[MAP_HEIGHT][MAP_WIDTH];

// assets.c
int game_assets_load(GameState* gs); // 1 = every required asset loaded
void game_assets_free(GameState* gs);
void game_reset(GameState* gs);      // new run / retry: state only, loads nothing

// physics.c
void load_player_info(GameState* gameState, float* px, float* py, float* pw, float* ph);
int animation_smoothness(int frame, const int frames);
void add_physics(GameState* gameState);
void detectPlayer(GameState* gameState, Map* map, int row, int col, float dt, float px, float py, float pw, float ph);
void updateGolem(Map* map, int row, int col, float dt);
void updateBoss(GameState* gameState, float dt);
int touchBoss(GameState* gameState);
void immortal_events(GameState* gameState);

// collision.c
void collision_detect_floor(GameState* gameState);
void collision_detect_map(GameState* gameState, Map* map);
void collision_correction(GameState* gameState, float px, float py, float pw, float ph, float bx, float by, float bw, float bh);

// map.c
void loadMap(Map* map, int arr[][MAP_WIDTH], float x, float y);
void drawMap(GameState* gameState, Map* map);
void drawLedges(GameState* gameState);
void drawTilesMap(GameState* gameState, const Map* map, SDL_Rect srcRect, SDL_Texture* texture, int row, int col, double angle, int flip);
void drawBackground(GameState* gameState);
void drawMap_SingleplayerMode(GameState* gameState);
void draw_key_map(GameState* gameState, int ledgeNumber);

// render.c
void do_render(GameState* gameState);
#endif
