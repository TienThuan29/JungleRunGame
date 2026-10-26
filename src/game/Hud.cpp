#include "game/Hud.h"
#include "game/GameAssets.h"

namespace jr {
namespace {
constexpr SDL_Color kWhite{ 255, 255, 255, 255 }, kYellow{ 255, 255, 73, 255 };

void fill(SDL_Renderer* r, SDL_Rect rect, Uint8 cr, Uint8 cg, Uint8 cb, Uint8 ca) {
	SDL_SetRenderDrawColor(r, cr, cg, cb, ca);
	SDL_RenderFillRect(r, &rect);
}

void centered(SDL_Renderer* r, SDL_Texture* t, int y) {
	SDL_Rect d = sdl::sized(t, 0, y);
	d.x = (cfg::kWindowW - d.w) / 2;
	SDL_RenderCopy(r, t, nullptr, &d);
}
}

std::string Hud::clock(Uint32 ms) {
	char buf[32];
	SDL_snprintf(buf, sizeof buf, "%02u:%02u.%u", ms / 60000, ms / 1000 % 60, ms / 100 % 10);
	return buf;
}

void Hud::label(SDL_Renderer* r, TTF_Font* font, sdl::TexturePtr& tex, int& cached, int value) {
	if (tex && cached == value) return;
	tex = sdl::renderText(r, font, std::to_string(value), kWhite);
	cached = value;
}

void Hud::drawStatus(SDL_Renderer* r, const GameAssets& a, const HudInfo& h) {
	label(r, a.hudFont(), lives_, cachedLives_, h.lives);
	label(r, a.hudFont(), potions_, cachedPotions_, h.potions);
	const SDL_Rect t1 = sdl::sized(lives_.get(), 5, -15);
	const SDL_Rect t2 = sdl::sized(potions_.get(), 200, -15);
	const SDL_Rect src1{ animFrame(6, cfg::kItemFrames) * 32, 1 * 32, 32, 32 };
	const SDL_Rect src2{ animFrame(8, cfg::kItemFrames) * 32, 9 * 32, 32, 32 };
	const SDL_Rect icon1{ t1.x + t1.w, 0, 48, 48 }, icon2{ t2.x + t2.w, 0, 48, 48 };
	SDL_RenderCopy(r, lives_.get(), nullptr, &t1);
	SDL_RenderCopy(r, a.tex(TextureId::Items), &src1, &icon1);
	SDL_RenderCopy(r, potions_.get(), nullptr, &t2);
	SDL_RenderCopy(r, a.tex(TextureId::Items), &src2, &icon2);
	if (h.key) {
		const SDL_Rect src{ 0, 13 * 32, 32, 32 }, d{ icon2.x + 80, 0, 48, 48 };
		SDL_RenderCopy(r, a.tex(TextureId::Items), &src, &d);
	}

	if (!timer_ || cachedTenths_ != h.tenths) {
		timer_ = sdl::renderText(r, a.hudFont(), clock(static_cast<Uint32>(h.tenths) * 100), kWhite);
		cachedTenths_ = h.tenths;
	}
	SDL_Rect tt = sdl::sized(timer_.get(), 0, -15);
	tt.x = cfg::kWindowW - tt.w - 12;
	SDL_RenderCopy(r, timer_.get(), nullptr, &tt);

	// progress to the finish / arena: white = you, red = boss
	constexpr int x0 = 12, w = cfg::kWindowW - 24, y = 56;
	fill(r, { x0, y, w, 6 }, 0, 0, 0, 140);
	fill(r, { x0, y, static_cast<int>(w * SDL_clamp(h.progress, 0.0f, 1.0f)), 6 }, 255, 255, 255, 200);
	fill(r, { x0 + static_cast<int>(w * SDL_clamp(h.bossProgress, 0.0f, 1.0f)) - 2, y - 4, 5, 14 }, 230, 40, 40, 255);
	fill(r, { x0 + w - 3, y - 4, 4, 14 }, 255, 220, 60, 255);

	if (h.bossWarn) {  // red triangle on the left edge
		for (int i = 0; i < 26; ++i) fill(r, { 10 + i, 400 - (26 - i), 1, 2 * (26 - i) }, 230, 30, 30, 230);
	}
	if (h.bossHp > 0 && h.bossMaxHp > 0) {
		constexpr int bw = 400, bx = (cfg::kWindowW - bw) / 2, by = 76;
		fill(r, { bx - 3, by - 3, bw + 6, 20 }, 0, 0, 0, 160);
		const int seg = bw / h.bossMaxHp;
		for (int i = 0; i < h.bossHp; ++i) fill(r, { bx + i * seg, by, seg - 4, 14 }, 220, 40, 40, 255);
	}
	if (h.god) {
		if (!god_) god_ = sdl::renderText(r, a.smallFont(), "TEST: invulnerable (F1)", kYellow);
		const SDL_Rect d = sdl::sized(god_.get(), 12, cfg::kWindowH - 40);
		SDL_RenderCopy(r, god_.get(), nullptr, &d);
	}
	SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
}

void Hud::resetOver() {
	overTitle_.reset();
	overStats_.reset();
	overBest_.reset();
	overHint_.reset();
}

void Hud::drawOver(SDL_Renderer* r, const GameAssets& a, const char* title, SDL_Color fg, bool showHint, const OverInfo& o) {
	if (!overTitle_) {
		overTitle_ = sdl::renderText(r, a.overFont(), title, fg);
		overStats_ = sdl::renderText(r, a.smallFont(), "Time " + clock(o.timeMs) + "      Potions " + std::to_string(o.potions) +
			"/" + std::to_string(cfg::kRequiredPotions) + "      Respawns " + std::to_string(o.respawns), kWhite);
		if (o.newRecord) overBest_ = sdl::renderText(r, a.smallFont(), "NEW RECORD!", kYellow);
		else if (o.bestMs > 0) overBest_ = sdl::renderText(r, a.smallFont(), "Best " + clock(o.bestMs), kYellow);
		overHint_ = sdl::renderText(r, a.hudFont(), o.canNext ? "R retry    X menu    N next world" : "R retry    X menu", kYellow);
	}
	SDL_Rect t = sdl::sized(overTitle_.get(), 0, 0);
	t.x = (cfg::kWindowW - t.w) / 2;
	t.y = (cfg::kWindowH - t.h) / 2 - 60;
	SDL_RenderCopy(r, overTitle_.get(), nullptr, &t);
	centered(r, overStats_.get(), t.y + t.h + 10);
	if (overBest_) centered(r, overBest_.get(), t.y + t.h + 50);
	if (showHint) centered(r, overHint_.get(), cfg::kWindowH - 110);
}
}
