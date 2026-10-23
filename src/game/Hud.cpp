#include "game/Hud.h"
#include "game/GameAssets.h"

namespace jr {
void Hud::label(SDL_Renderer* r, TTF_Font* font, sdl::TexturePtr& tex, int& cached, int value) {
	if (tex && cached == value) return;
	tex = sdl::renderText(r, font, std::to_string(value), SDL_Color{ 255, 255, 255, 255 });
	cached = value;
}

void Hud::drawStatus(SDL_Renderer* r, const GameAssets& a, int lives, int potions) {
	label(r, a.hudFont(), lives_, cachedLives_, lives);
	label(r, a.hudFont(), potions_, cachedPotions_, potions);
	const SDL_Rect t1 = sdl::sized(lives_.get(), 5, -15);
	const SDL_Rect t2 = sdl::sized(potions_.get(), 200, -15);
	const SDL_Rect src1{ animFrame(6, cfg::kItemFrames) * 32, 1 * 32, 32, 32 };
	const SDL_Rect src2{ animFrame(8, cfg::kItemFrames) * 32, 9 * 32, 32, 32 };
	const SDL_Rect icon1{ t1.x + t1.w, 0, 48, 48 }, icon2{ t2.x + t2.w, 0, 48, 48 };
	SDL_RenderCopy(r, lives_.get(), nullptr, &t1);
	SDL_RenderCopy(r, a.tex(TextureId::Items), &src1, &icon1);
	SDL_RenderCopy(r, potions_.get(), nullptr, &t2);
	SDL_RenderCopy(r, a.tex(TextureId::Items), &src2, &icon2);
}

void Hud::drawOver(SDL_Renderer* r, const GameAssets& a, const char* title, SDL_Color fg, bool showHint) {
	if (!overTitle_) overTitle_ = sdl::renderText(r, a.overFont(), title, fg);
	SDL_Rect t = sdl::sized(overTitle_.get(), 0, 0);
	t.x = (cfg::kWindowW - t.w) / 2;
	t.y = (cfg::kWindowH - t.h) / 2;
	SDL_RenderCopy(r, overTitle_.get(), nullptr, &t);
	if (!showHint) return;
	if (!overHint_) overHint_ = sdl::renderText(r, a.hudFont(), "Press R to retry, X to return menu", SDL_Color{ 255, 255, 73, 255 });
	const SDL_Rect h = sdl::sized(overHint_.get(), 0, -15);
	SDL_RenderCopy(r, overHint_.get(), nullptr, &h);
}
}
