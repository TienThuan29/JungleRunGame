#include "game/Player.h"
#include "game/GameAssets.h"
#include <cmath>

namespace jr {
void Player::reset(int lives) {
	st_ = PlayerState{};
	st_.lives = lives;
}

void Player::input(const InputState& in) {
	if (in.left) {
		st_.status = 1;
		st_.flip = 1;
		st_.dx -= 0.3f;
		if (st_.dx < -2.5f) st_.dx = -2.5f;
	}
	else if (in.right) {
		st_.status = 1;
		st_.flip = 0;
		st_.dx += 0.3f;
		if (st_.dx > 2.5f) st_.dx = 2.5f;
	}
	else if (in.down) {
		st_.dy += 0.3f;
	}
	else {
		st_.status = 0;
		st_.flip = 0;
		st_.dx *= 0.8f;
		if (std::fabs(st_.dx) < 0.1f) st_.dx = 0;
	}
}

void Player::jump() {
	if (st_.onLedge) {
		st_.onLedge = 0;
		st_.dy = -9;
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

void Player::hurt(Mix_Chunk* sound) {
	st_.isTakenDamage = 1;
	st_.lives--;
	st_.isImmortal = 1;
	st_.immortalStartTime = static_cast<float>(SDL_GetTicks64()) / 1000.0f;
	Mix_PlayChannel(-1, sound, 0);
}

void Player::heal(int maxLives) {
	st_.lives++;
	if (st_.lives > maxLives) st_.lives = maxLives;
}

void Player::updateImmortal(float nowSec) {
	if (nowSec - st_.immortalStartTime >= 1.0f) st_.isImmortal = 0;
}

void Player::integrate(Mix_Chunk* jumpSound) {
	st_.x += st_.dx;
	st_.y += st_.dy;
	st_.dy += cfg::kGravity;
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

void Player::draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const {
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
		const SDL_Rect d{ x, y, w * 2, h * 2 };
		SDL_RenderCopyEx(r, t, nullptr, &d, 0, nullptr, flip);
	}
}
}
