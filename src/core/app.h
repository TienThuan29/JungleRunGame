#ifndef APP_H
#define APP_H
#include <SDL.h>
#include <SDL_mixer.h>

#define MENU_W 960
#define MENU_H 668

typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;
	Mix_Chunk* click;      // menu click sound, shared by all menu scenes
	Mix_Music* menuMusic;  // keeps playing across menu scenes
	int difficulty;        // DIFFICULTY_EASY / MEDIUM / HARD
	int modeIndex;         // 0 Easy, 1 Medium, 2 Hard (Game Mode label)
} App;

int app_init(App* app); // 1 = ok
void app_quit(App* app);
void app_resize(App* app, int w, int h);
void app_tick(App* app); // once per loop iteration; Debug: live counts in title every second
void app_fatal(App* app, const char* msg);
#endif
