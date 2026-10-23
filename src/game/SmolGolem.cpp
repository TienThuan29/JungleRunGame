#include "game/SmolGolem.h"
#include "game/GameAssets.h"
#include "game/Player.h"
#include <cmath>

namespace jr {
void SmolGolem::preCollide(const Player& p, float dt) {
	float px, py, pw, ph;
	p.hitInfo(px, py, pw, ph);
	const float bw = static_cast<float>(cfg::kTile), bx = box_.x, by = box_.y;
	if (px + pw >= bx - cfg::kSmolGolemDetect && px <= bx + bw + cfg::kSmolGolemDetect &&
		std::fabs(static_cast<double>(by - py)) <= 2 * cfg::kTile) {
		if (px < bx + (bw / 2)) {
			dx_ += -cfg::kSmolGolemSpeed;
			flip_ = 1;
		}
		else if (px > bx + (bw / 2)) {
			dx_ += cfg::kSmolGolemSpeed;
			flip_ = 0;
		}
	}
	else {
		dx_ = 0.f;
	}
	if (dx_ > 120) dx_ = 120;
	if (dx_ < -120) dx_ = -120;
	box_.x += dx_ * dt;
}

void SmolGolem::draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const {
	const SDL_Rect src{ 24 + animFrame(8, cfg::kSmolGolemFrames) * 100, 124, 50, 50 };
	const SDL_Rect d = dst(scrollX);
	SDL_RenderCopyEx(r, a.tex(TextureId::Golem), &src, &d, 0, nullptr, flip_ ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}
}
