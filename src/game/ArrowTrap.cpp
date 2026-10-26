#include "game/ArrowTrap.h"
#include "game/GameAssets.h"
#include "game/World.h"
#include <cmath>

namespace jr {
void ArrowTrap::update(const Player& p, World& w, float dt) {
	const float period = w.tuning().arrowEverySec;
	t_ = std::fmod(t_ + dt, period);
	const int f = SDL_min(kFrames - 1, static_cast<int>(t_ / period * kFrames));
	const bool fire = frame_ < kFireFrame && f >= kFireFrame;
	frame_ = f;
	if (!fire || std::fabs(p.state().x - box_.x) > 1300) return;  // only near the player
	const float speed = w.tuning().arrowSpeed;
	const float y = box_.y + 33;  // arrow height in the 32 px frame (21) scaled to the 50 px cell
	if (right_) w.spawnProjectile(ProjectileKind::Arrow, box_.x + box_.w - 10, y, speed, 0);
	else w.spawnProjectile(ProjectileKind::Arrow, box_.x - 48, y, -speed, 0);
}

void ArrowTrap::draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const {
	const SDL_Rect src{ frame_ * 96 + 32, 0, 32, 32 };
	const SDL_Rect d = dst(scrollX);
	SDL_RenderCopyEx(r, a.tex(TextureId::ArrowTrap), &src, &d, 0, nullptr, right_ ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}
}
