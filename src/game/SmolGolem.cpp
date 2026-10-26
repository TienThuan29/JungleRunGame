#include "game/SmolGolem.h"
#include "game/GameAssets.h"
#include "game/World.h"
#include <cmath>

namespace jr {
void SmolGolem::update(const Player& p, World& w, float dt) {
	if (dying_) {
		deathT_ += dt;
		return;
	}
	const Tuning& t = w.tuning();
	const float mul = fast_ ? t.smolFastMul : 1.0f;
	float px, py, pw, ph;
	p.hitInfo(px, py, pw, ph);
	const float bw = static_cast<float>(cfg::kTile), bx = box_.x, by = box_.y;
	if (px + pw >= bx - t.smolDetect && px <= bx + bw + t.smolDetect &&
		std::fabs(static_cast<double>(by - py)) <= 2 * cfg::kTile) {
		if (px < bx + (bw / 2)) {
			dx_ -= t.smolSpeed * mul;
			flip_ = 1;
		}
		else if (px > bx + (bw / 2)) {
			dx_ += t.smolSpeed * mul;
			flip_ = 0;
		}
	}
	else {
		dx_ = 0.f;
	}
	const float max = t.smolMax * mul;
	if (dx_ > max) dx_ = max;
	if (dx_ < -max) dx_ = -max;
	box_.x += dx_ * dt;
}

bool SmolGolem::hit(World& w) {
	if (dying_) return false;
	dying_ = true;
	w.particles().burst(box_.x + box_.w / 2, box_.y + box_.h / 2, 14, SDL_Color{ 150, 150, 150, 255 }, 160);
	Mix_PlayChannel(-1, w.assets().hurt(HurtSound::Spike), 0);
	return true;
}

void SmolGolem::draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const {
	SDL_Texture* t = a.tex(TextureId::Golem);
	SDL_Rect src{ 24 + animFrame(8, cfg::kSmolGolemFrames) * 100, 124, 50, 50 };
	if (dying_) {  // crumble: row 7 (10 frames) then row 8 (4 frames)
		const int f = SDL_min(13, static_cast<int>(deathT_ / kDeathSec * 14));
		src = f < 10 ? SDL_Rect{ 24 + f * 100, 724, 50, 50 } : SDL_Rect{ 24 + (f - 10) * 100, 824, 50, 50 };
	}
	const SDL_Rect d = dst(scrollX);
	if (fast_) SDL_SetTextureColorMod(t, 255, 160, 80);
	SDL_RenderCopyEx(r, t, &src, &d, 0, nullptr, flip_ ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
	if (fast_) SDL_SetTextureColorMod(t, 255, 255, 255);
}
}
