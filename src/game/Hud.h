#pragma once
#include "core/SdlHandles.h"
#include <string>

namespace jr {
class GameAssets;

struct HudInfo {
	int lives = 0, potions = 0;
	bool key = false;
	int tenths = 0;                        // run time in 1/10 s
	float progress = 0, bossProgress = 0;  // 0..1 toward the goal (finish or arena)
	bool bossWarn = false;                 // boss off screen but close behind (already blinking)
	int bossHp = 0, bossMaxHp = 0;         // bossHp > 0: boss fight bar
	bool god = false;                      // test key F1
};

struct OverInfo {
	Uint32 timeMs = 0, bestMs = 0;
	int potions = 0, respawns = 0;
	bool newRecord = false, canNext = false;
};

// Status bar and ending screen. Text textures are rebuilt only when their value changes.
class Hud {
public:
	void drawStatus(SDL_Renderer* r, const GameAssets& a, const HudInfo& h);
	void drawOver(SDL_Renderer* r, const GameAssets& a, const char* title, SDL_Color fg, bool showHint, const OverInfo& o);
	void resetOver();
	static std::string clock(Uint32 ms);  // mm:ss.t
private:
	static void label(SDL_Renderer* r, TTF_Font* font, sdl::TexturePtr& tex, int& cached, int value);
	sdl::TexturePtr lives_, potions_, timer_, god_, overTitle_, overStats_, overBest_, overHint_;
	int cachedLives_ = 0, cachedPotions_ = 0, cachedTenths_ = -1;
};
}
