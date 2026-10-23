#pragma once
#include "core/SdlHandles.h"

namespace jr {
class GameAssets;

// Lives / potion counters and the ending-screen text. Text textures are rebuilt only when needed.
class Hud {
public:
	void drawStatus(SDL_Renderer* r, const GameAssets& a, int lives, int potions);  // = render_status()
	void drawOver(SDL_Renderer* r, const GameAssets& a, const char* title, SDL_Color fg, bool showHint);
	void resetOver() {
		overTitle_.reset();
		overHint_.reset();
	}
private:
	static void label(SDL_Renderer* r, TTF_Font* font, sdl::TexturePtr& tex, int& cached, int value);
	sdl::TexturePtr lives_, potions_, overTitle_, overHint_;
	int cachedLives_ = 0, cachedPotions_ = 0;
};
}
