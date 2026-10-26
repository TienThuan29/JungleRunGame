#include "game/Player.h"
#include "game/GameAssets.h"
#include <algorithm>
#include <cmath>

namespace jr {
void Player::reset(int lives, const Tuning& t) {
	t_ = &t;
	st_ = PlayerState{};
	st_.lives = lives;
	attackStart_ = -1;
	attackUntil_ = nextAttack_ = 0;
}

void Player::input(const InputState& in) {
	if (in.left) {
		st_.status = 1;
		st_.flip = 1;
		st_.dx -= t_->runAccel;
		if (st_.dx < -t_->runMax) st_.dx = -t_->runMax;
	}
	else if (in.right) {
		st_.status = 1;
		st_.flip = 0;
		st_.dx += t_->runAccel;
		if (st_.dx > t_->runMax) st_.dx = t_->runMax;
	}
	else if (in.down) {
		st_.dy += t_->fallAccel;
	}
	else {
		st_.status = 0;
		st_.dx *= t_->friction;
		if (std::fabs(st_.dx) < 0.1f) st_.dx = 0;
	}
}

void Player::jump() {
	if (st_.onLedge) {
		st_.onLedge = 0;
		st_.dy = t_->jumpVy;
	}
}

void Player::hitInfo(float& px, float& py, float& pw, float& ph) const {
	pw = cfg::kIdleW * 2, ph = cfg::kIdleH * 2;
	if (st_.status == 2 || st_.status == 3 || st_.status == 4) {
		pw = cfg::kJumpW * 2, ph = cfg::kJumpH * 2;
	}
	px = st_.x, py = st_.y;
}

SDL_Rect Player::hitRect() const {  // = rectA in collision_detect_map (float -> int truncation)
	float px, py, pw, ph;
	hitInfo(px, py, pw, ph);
	return { static_cast<int>(px), static_cast<int>(py), static_cast<int>(pw), static_cast<int>(ph) };
}

void Player::resolveAgainst(const SDL_FRect& b, Mix_Chunk* landing) {
	float px, py, pw, ph;
	hitInfo(px, py, pw, ph);
	const float bx = b.x, by = b.y, bw = b.w, bh = b.h;
	if (py + 0.2 * ph > by && py < by + bh) {
		if (px < (bx + bw) && (px + pw) > (bx + bw) && st_.dx < 0) {
			st_.x = bx + bw;
			st_.dx = 0;
		}
		else if ((px + pw) > bx && px < bx && st_.dx > 0) {
			st_.x = bx - pw;
			st_.dx = 0;
		}
	}
	if (px + pw / 2 > bx && px + pw / 2 < bx + bw) {
		if (py < (by + bh) && py > by && st_.dy < 0) {
			st_.y = by + bh;
			st_.dy = 0;
		}
	}
	if (px + pw > bx && px < bx + bw) {
		if (py + ph > by && py < by && st_.dy > 0) {
			st_.y = by - ph;
			if (!st_.onLedge) Mix_PlayChannel(-1, landing, 0);
			st_.dy = 0;
			st_.onLedge = 1;
		}
	}
}

void Player::hurt(Mix_Chunk* sound, float now) {
	st_.isTakenDamage = 1;
	st_.lives--;
	st_.isImmortal = 1;
	st_.immortalStartTime = now;
	st_.immortalSec = t_->immortalSec;
	Mix_PlayChannel(-1, sound, 0);
}

void Player::heal(int maxLives) {
	st_.lives++;
	if (st_.lives > maxLives) st_.lives = maxLives;
}

void Player::updateImmortal(float now) {
	if (now - st_.immortalStartTime >= st_.immortalSec) st_.isImmortal = 0;
}

void Player::integrate(Mix_Chunk* jumpSound) {
	st_.x += st_.dx;
	st_.y += st_.dy;
	st_.dy += t_->gravity;
	if (!st_.onLedge) {
		if (st_.dy < 0) {
			st_.status = 2;
			if (!Mix_Playing(1)) Mix_PlayChannel(1, jumpSound, 0);
		}
		else if (st_.dy == 0) {
			st_.status = 3;
		}
		else {
			st_.status = 4;
		}
	}
}

void Player::respawn(float x, float y, int lives, float now) {
	const int flip = st_.flip;
	st_ = PlayerState{};
	st_.x = x;
	st_.y = y;
	st_.flip = flip;
	st_.lives = lives;
	st_.isImmortal = 1;
	st_.immortalStartTime = now;
	st_.immortalSec = t_->respawnImmortalSec;
	attackUntil_ = nextAttack_ = 0;
}

void Player::confine(float minX, float maxX) {
	float px, py, pw, ph;
	hitInfo(px, py, pw, ph);
	if (st_.x < minX) {
		st_.x = minX;
		if (st_.dx < 0) st_.dx = 0;
	}
	else if (st_.x + pw > maxX) {
		st_.x = maxX - pw;
		if (st_.dx > 0) st_.dx = 0;
	}
}

bool Player::attack(float now, Mix_Chunk* swing) {
	if (now < nextAttack_) return false;
	attackStart_ = now;
	attackUntil_ = now + t_->attackSec;
	nextAttack_ = now + t_->attackCooldownSec;
	++swing_;
	Mix_PlayChannel(-1, swing, 0);
	return true;
}

SDL_Rect Player::attackRect() const {
	float px, py, pw, ph;
	hitInfo(px, py, pw, ph);
	const int w = static_cast<int>(t_->attackW), h = static_cast<int>(t_->attackH);
	const int x = st_.flip ? static_cast<int>(px) - w : static_cast<int>(px + pw);
	return { x, static_cast<int>(py + ph / 2) - h / 2, w, h };
}

void Player::draw(SDL_Renderer* r, const GameAssets& a, float scrollX, float now) const {
	SDL_Texture* t = nullptr;
	int w = cfg::kJumpW, h = cfg::kJumpH;
	switch (st_.status) {
	case 0: t = a.idle(animFrame(12, cfg::kCharacterFrames)); w = cfg::kIdleW; h = cfg::kIdleH; break;
	case 1: t = a.run(animFrame(8, cfg::kCharacterFrames)); w = cfg::kRunW; h = cfg::kRunH; break;
	case 2: t = a.jump(0); break;
	case 3: t = a.jump(1); break;
	case 4: t = a.jump(animFrame(2, cfg::kCharacterFrames) + 2); break;
	default: return;
	}
	const int x = static_cast<int>(scrollX + st_.x), y = static_cast<int>(st_.y);
	const SDL_RendererFlip flip = st_.flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	if (st_.isTakenDamage && st_.isImmortal) {  // blink red, natural size x2 (render.c draw_player_hit)
		SDL_Rect d{ x, y, 0, 0 };
		SDL_QueryTexture(t, nullptr, nullptr, &d.w, &d.h);
		d.w *= 2;
		d.h *= 2;
		const bool red = (SDL_GetTicks64() / 100) % 2 != 0;
		if (red) SDL_SetTextureColorMod(t, 255, 0, 0);
		SDL_RenderCopyEx(r, t, nullptr, &d, 0, nullptr, flip);
		if (red) SDL_SetTextureColorMod(t, 255, 255, 255);
	}
	else {
		const bool blink = st_.isImmortal && (SDL_GetTicks64() / 80) % 2 != 0;  // respawn invulnerability
		if (blink) SDL_SetTextureAlphaMod(t, 110);
		const SDL_Rect d{ x, y, w * 2, h * 2 };
		SDL_RenderCopyEx(r, t, nullptr, &d, 0, nullptr, flip);
		if (blink) SDL_SetTextureAlphaMod(t, 255);
	}
	if (attacking(now)) {  // slash sheet: 3x3 frames of 64x47, frame 0 is a speck -> use 1..7
		const float k = (now - attackStart_) / t_->attackSec;
		const int f = 1 + std::clamp(static_cast<int>(k * 7), 0, 6);
		const SDL_Rect src{ f % 3 * 64, f / 3 * 47, 64, 47 };
		const SDL_Rect ar = attackRect();
		const SDL_Rect d{ static_cast<int>(scrollX) + ar.x + ar.w / 2 - 48, ar.y + ar.h / 2 - 35, 96, 70 };
		SDL_RenderCopyEx(r, a.tex(TextureId::Slash), &src, &d, 0, nullptr, flip);
	}
}
}
