#pragma once
#include "game/GameAssets.h"
#include "game/Player.h"

namespace jr {
// The boss chasing the player from the left.
class GigaGolem {
public:
	void reset() {
		x_ = 0;
		y_ = (cfg::kWindowH - cfg::kGigaGolemSize) / 2.0f;
		dx_ = 0;
	}
	void update(float dt, float scrollX, float playerDx) {  // = updateBoss()
		const float camX = -scrollX;
		if (camX - x_ > 0 && playerDx > 0) dx_ += playerDx;
		else dx_ = static_cast<float>(cfg::kGigaGolemSpeed);
		x_ += dx_ * dt;
	}
	bool touches(const Player& p) const {  // = touchBoss()
		float px, py, pw, ph;
		p.hitInfo(px, py, pw, ph);
		return px <= (x_ + 0.75 * static_cast<float>(cfg::kGigaGolemSize));
	}
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const {
		const SDL_Rect src{ 24 + animFrame(10, cfg::kGigaGolemFrames) * 100, 624, 50, 50 };
		const SDL_Rect d{ static_cast<int>(scrollX + x_), static_cast<int>(y_), cfg::kGigaGolemSize, cfg::kGigaGolemSize };
		SDL_RenderCopyEx(r, a.tex(TextureId::Golem), &src, &d, 0, nullptr, SDL_FLIP_NONE);
	}
	float x() const { return x_; }
private:
	float x_ = 0, y_ = 0, dx_ = 0;
};
}
