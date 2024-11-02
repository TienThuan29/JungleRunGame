#ifndef STRUCTS
#define STRUCTS
#include "defs.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

typedef struct {
	float x;
	float y;
} Vector;

typedef struct {
	Vector pos;
	Vector d;
	int flip;
} Golem;

typedef struct {
	float x, y;
	float dx, dy;
	short lives;
	char* name;
	int isTakenDamage;
	// imomortal event
	int isImmortal;
	float immortalStartTime;
	int onLedge; // 0 = false, 1 = true
	int flip, status; // 0 = idle, 1 = run, 2 = jump, 3 = mid air, 4 = falling
} Object;

typedef struct {
	float x, y, w, h;
	// item info
	int isItem, itemType, isObtained;
	int isLethal; // 0 = false, 1 = true
	int isBlocked; // 0 = false, 1 = true
	Golem smolGolem;
} Ledge;

typedef struct {
	float x, y;// x, y position to start drawing map
	int counter; // number of maps created actually
	int pos[MAP_HEIGHT][MAP_WIDTH]; // tilesmap
	Ledge ledges[MAP_HEIGHT][MAP_WIDTH];
} Map;

typedef struct {
	float defaultX, previousX, nextX;
	int scrollSpeed;	
	SDL_Texture* layer[3];
} Background; // use to make parallax effect

typedef struct {
	Mix_Chunk* getDamaged;
	Mix_Chunk* electricHurt;
	Mix_Chunk* ItemPickUp;
	Mix_Chunk* landing;
	Mix_Chunk* jump;
	Mix_Chunk* spike;
} SoundEffect;

typedef struct {
	Mix_Music* inGame[3];
	Mix_Music* ending[3];
} Music;

typedef struct {
	// Time
	float dt, overTiming, bossStart;
	// difficulty
	int mode, difficulty, setDifficulty;
	// Required items to achieve good ending
	int health_potion_counter; // if = 5 -> good ending 
	// draw key map
	int isKeyObtained, flag;
	// Scroll offset
	float scrollX;
	// status state
	int statusState, isOver;
	// Giga golem
	Golem gigaGolem;
	// Players
	Object player;
	// Ledges
	Ledge ledges[NUM_OF_LEDGES];
	// Images
		// Trap
			// Fire trap
	SDL_Texture* fire_trap[3];
			// Ceiling trap
	SDL_Texture* ceiling_trap[1];
			// Saw trap
	SDL_Texture* saw_trap[1];
			// Lightning trap
	SDL_Texture* lightning_trap[2];
		// Items
	SDL_Texture* items[1];
		// Sparkling star
	SDL_Texture* sparkling_star[1];
		// Character animation
	SDL_Texture* idle_anim[12];
	SDL_Texture* jump_anim[4];
	SDL_Texture* run_anim[8];
		// Golem
	SDL_Texture* golem[1];
		// Platform: 
		// platform[5] is jungle tileset
	SDL_Texture* platform[6];
	// Background
	Background background[5];
	// Label
	SDL_Texture* label_lives;
	SDL_Texture* label_health_potion;
	// Fonts
	TTF_Font* font;
	// Sound
	SoundEffect soundEffects;
	Music soundTracks;
	// Renderer
	SDL_Renderer* renderer;
	// Map
	Map map[20];
} GameState;
#endif // !stucts