#include "game/GigaGolem.h"
#include "game/World.h"
#include <cmath>

namespace jr {
void GigaGolem::placeAt(float x) {
	x_ = x;
	y_ = (cfg::kWindowH - cfg::kGigaGolemSize) / 2.0f;
	dx_ = fireT_ = flash_ = 0;
	windup_ = false;
	enter(Phase::Chase, 1);
}

void GigaGolem::startArena(float arenaX, int hp) {
	arenaX_ = arenaX;
	x_ = arenaX + 40;
	y_ = cfg::kGroundTop - kFeet;
	hp_ = maxHp_ = hp;
	cycle_ = 0;
	flash_ = 0;
	windup_ = false;
	enter(Phase::Rise, kRiseSec);
}

void GigaGolem::update(World& w, float dt) {
	const Tuning& t = w.tuning();
	t_ += dt;
	if (flash_ > 0) flash_ -= dt;
	switch (phase_) {
	case Phase::Chase: {  // = updateBoss(), plus the arm throw
		const PlayerState& s = w.player().state();
		const float camX = -w.scrollX();
		if (camX - x_ > 0 && s.dx > 0) dx_ += s.dx;
		else dx_ = t.bossSpeed;
		x_ += dx_ * dt;
		if (!windup_) {
			fireT_ += dt;
			if (fireT_ >= t.bossFireSec && std::fabs(s.x - x_) < t.bossFireRange) {
				windup_ = true;
				t_ = 0;
			}
		}
		else if (t_ >= kWindupSec) {
			windup_ = false;
			fireT_ = 0;
			float px, py, pw, ph;
			w.player().hitInfo(px, py, pw, ph);
			const float sx = x_ + kTipX, sy = y_ + kTipY;
			const float vx = px + pw / 2 - sx, vy = py + ph / 2 - sy;
			const float len = SDL_max(1.0f, std::sqrt(vx * vx + vy * vy));
			w.spawnProjectile(ProjectileKind::Arm, sx - 32, sy - 12, vx / len * t.armSpeed, vy / len * t.armSpeed);
		}
		break;
	}
	case Phase::Rise:
		if (t_ >= len_) enter(Phase::Idle, t.bossIdleSec);
		break;
	case Phase::Idle:
		if (t_ < len_) break;
		if (cycle_++ % 2 == 0) {
			enter(Phase::Charge, t.bossChargeSec);
			Mix_PlayChannel(-1, w.assets().hurt(HurtSound::Electric), 0);  // warning
		}
		else {
			enter(Phase::Throw, kThrowSec);
		}
		break;
	case Phase::Charge:
		if (t_ >= len_) enter(Phase::Beam, t.bossBeamSec);
		break;
	case Phase::Beam:
		if (t_ >= len_) enter(Phase::Open, t.bossOpenSec);
		break;
	case Phase::Throw:
		if (t_ - dt < kThrowAt && t_ >= kThrowAt) {
			w.spawnProjectile(ProjectileKind::Arm, x_ + kBodyX + kBodyW, cfg::kGroundTop - 40.0f, t.armSpeed, 0);
		}
		if (t_ >= len_) enter(Phase::Open, t.bossOpenSec);
		break;
	case Phase::Open:
		if (t_ >= len_) enter(Phase::Idle, t.bossIdleSec);
		break;
	case Phase::Dying:
		if (t_ >= len_) enter(Phase::Dead, 1);
		break;
	case Phase::Dead:
		break;
	}
}

bool GigaGolem::touches(const Player& p) const {  // = touchBoss()
	float px, py, pw, ph;
	p.hitInfo(px, py, pw, ph);
	return px <= right();
}

SDL_FRect GigaGolem::body() const { return { x_ + kBodyX, y_ + 10, kBodyW, kFeet - 10 }; }

SDL_FRect GigaGolem::beam() const {
	const float left = x_ + kBodyX + kBodyW;
	return { left, y_ + kTipY - 15, arenaX_ + cfg::kWindowW - left, 30 };
}

bool GigaGolem::hitBySword(World& w) {
	if (phase_ != Phase::Open) return false;
	flash_ = 0.2f;
	Mix_PlayChannel(-1, w.assets().hurt(HurtSound::Damage), 0);
	if (--hp_ <= 0) enter(Phase::Dying, kDyingSec);
	return true;
}

int GigaGolem::progress(int frames) const {
	return SDL_min(frames - 1, static_cast<int>(t_ / len_ * static_cast<float>(frames)));
}

void GigaGolem::drawCell(SDL_Renderer* r, SDL_Texture* t, int row, int frame, float scrollX) const {
	const SDL_Rect src{ 24 + frame * 100, row * 100 + 24, 76, 50 };
	const SDL_Rect d{ static_cast<int>(scrollX + x_), static_cast<int>(y_), static_cast<int>(kW), static_cast<int>(kH) };
	SDL_RenderCopy(r, t, &src, &d);
}

void GigaGolem::drawCrumble(SDL_Renderer* r, SDL_Texture* t, int f, float scrollX) const {
	if (f < 10) drawCell(r, t, 7, f, scrollX);
	else drawCell(r, t, 8, f - 10, scrollX);
}

void GigaGolem::draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const {
	SDL_Texture* t = a.tex(TextureId::Golem);
	const bool red = flash_ > 0;
	if (red) SDL_SetTextureColorMod(t, 255, 70, 70);
	const int tipX = static_cast<int>(scrollX + x_ + kTipX), tipY = static_cast<int>(y_ + kTipY);
	SDL_Texture* laser = a.tex(TextureId::Laser);
	switch (phase_) {
	case Phase::Chase:
		if (windup_) drawCell(r, t, 2, SDL_min(7, static_cast<int>(t_ / kWindupSec * 8)), scrollX);
		else drawCell(r, t, 6, animFrame(10, cfg::kGigaGolemFrames), scrollX);
		break;
	case Phase::Rise: drawCrumble(r, t, 13 - progress(14), scrollX); break;
	case Phase::Idle: drawCell(r, t, 0, animFrame(4, 150), scrollX); break;
	case Phase::Charge: {
		const int f = progress(8);
		drawCell(r, t, 2, f, scrollX);
		if (f >= 4) {  // energy ball grows at the arm tip (laser rows 1-8, ball centre at (52, 33))
			const int lf = 1 + progress(8);
			const SDL_Rect src{ 0, lf * 100, 100, 100 }, d{ tipX - 104, tipY - 66, 200, 200 };
			SDL_RenderCopy(r, laser, &src, &d);
		}
		break;
	}
	case Phase::Beam: {
		drawCell(r, t, 2, 7, scrollX);
		const int row = 9 + animFrame(6, 60);  // beam rows 9-14: ball at x 30-55, beam to x 300
		const SDL_Rect first{ 0, row * 100, 300, 100 }, rest{ 150, row * 100, 150, 100 };
		SDL_Rect d{ tipX - 52, tipY - 33, 300, 100 };
		SDL_RenderCopy(r, laser, &first, &d);
		const int end = static_cast<int>(scrollX + arenaX_) + cfg::kWindowW;
		for (d.x += 300, d.w = 150; d.x < end; d.x += 150) SDL_RenderCopy(r, laser, &rest, &d);
		break;
	}
	case Phase::Throw: drawCell(r, t, 4, progress(7), scrollX); break;
	case Phase::Open: drawCell(r, t, 5, animFrame(7, 120), scrollX); break;
	case Phase::Dying: drawCrumble(r, t, progress(14), scrollX); break;
	case Phase::Dead: break;
	}
	if (red) SDL_SetTextureColorMod(t, 255, 255, 255);
}
}
