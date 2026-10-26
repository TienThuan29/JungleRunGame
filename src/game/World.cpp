#include "game/World.h"
#include <algorithm>
#include <cmath>

namespace jr {
namespace {
constexpr size_t kMaxProjectiles = 64;
constexpr float kProjectileLife = 4.0f;
constexpr float kArenaTrigger = 450;  // the fight starts this far inside, so the boss rises behind the player

SDL_Rect toRect(const SDL_FRect& f) { return toIntRect(f); }
}

void World::reset() {
	status_ = Status::Playing;
	isOver_ = keyObtained_ = swapped_ = arena_ = hasCheckpoint_ = false;
	overTiming_ = 0;
	ticks_ = potions_ = respawns_ = bossHitSwing_ = 0;
	endMs_ = 0;
	shake_ = 0;
	over_ = OverInfo{};
	player_.reset(difficulty_, tuning_);
	bossStart_ = 0;
	boss_.reset();
	scrollX_ = 0;
	parallax_.reset();
	hud_.resetOver();
	particles_.clear();
	projectiles_.clear();
	level_.build(course_, difficulty_ != cfg::kDifficultyHard);
}

void World::tick(const InputState& in) {
	++ticks_;
	const float dt = cfg::kStep;
	player_.input(in);
	level_.collideGround(player_, assets_.landing());
	level_.collideSegments(player_, *this, dt);
	if (!isOver_) combat();
	physics();
	level_.prune();
	level_.animate(static_cast<Uint64>(ticks_) * 1000 / 60);  // traps: lethal/solid flags for the next tick
	if (keyObtained_ && !swapped_) {
		level_.swapOnKey(course_);
		swapped_ = true;
	}
	particles_.update(dt);
	shake_ = SDL_max(0.0f, shake_ - tuning_.shakeDecay * dt);
}

void World::attack() {
	if (!isOver_) player_.attack(time(), assets_.swing());
}

void World::setCheckpoint(float x, float feetY) {
	if (hasCheckpoint_ && x <= cpX_) return;  // an earlier, skipped star never moves the respawn point back
	hasCheckpoint_ = true;
	cpX_ = x;
	cpFeet_ = feetY;
}

void World::hurtPlayer(HurtSound s) {
	if (god_ || isOver_ || player_.state().isImmortal) return;
	player_.hurt(assets_.hurt(s), time());
	shake(tuning_.shakeHit);
	const SDL_Rect p = player_.hitRect();
	particles_.burst(static_cast<float>(p.x + p.w / 2), static_cast<float>(p.y + p.h / 2), 10, SDL_Color{ 230, 40, 40, 255 }, 150);
}

void World::spawnProjectile(ProjectileKind k, float x, float y, float dx, float dy) {
	if (projectiles_.size() >= kMaxProjectiles) return;
	const SDL_FRect box = k == ProjectileKind::Arm ? SDL_FRect{ x, y, 64, 24 } : SDL_FRect{ x, y, 58, 10 };
	projectiles_.push_back({ k, box, dx, dy, kProjectileLife });
}

void World::combat() {
	const float dt = cfg::kStep, now = time();
	const SDL_Rect pr = player_.hitRect();
	const bool swinging = player_.attacking(now);
	const SDL_Rect ar = swinging ? player_.attackRect() : SDL_Rect{};

	if (swinging) {
		level_.hitEntities(ar, *this);
		const SDL_Rect body = toRect(boss_.body());
		if (arena_ && bossHitSwing_ != player_.swing() && SDL_HasIntersection(&ar, &body) && boss_.hitBySword(*this)) {
			bossHitSwing_ = player_.swing();
			shake(tuning_.shakeHit);
			particles_.burst(static_cast<float>(ar.x + ar.w / 2), static_cast<float>(ar.y + ar.h / 2), 16, SDL_Color{ 90, 230, 255, 255 }, 200);
		}
	}

	for (Projectile& p : projectiles_) {
		p.box.x += p.dx * dt;
		p.box.y += p.dy * dt;
		p.life -= dt;
		const SDL_Rect b = toRect(p.box);
		if (swinging && SDL_HasIntersection(&ar, &b)) {  // cut it down
			p.life = 0;
			particles_.burst(p.box.x + p.box.w / 2, p.box.y + p.box.h / 2, 8, SDL_Color{ 255, 255, 255, 255 }, 120);
		}
		else if (SDL_HasIntersection(&pr, &b)) {
			hurtPlayer(HurtSound::Damage);
			p.life = 0;
		}
		if (p.box.y > cfg::kWindowH || p.box.y < -200) p.life = 0;
	}
	projectiles_.erase(std::remove_if(projectiles_.begin(), projectiles_.end(), [](const Projectile& p) { return p.life <= 0; }),
		projectiles_.end());

	if (!arena_ || !boss_.fighting()) return;
	const SDL_Rect body = toRect(boss_.body());
	if (SDL_HasIntersection(&pr, &body)) {  // walked into the boss: hurt and pushed out to the right
		hurtPlayer(HurtSound::Damage);
		player_.knockback(boss_.body().x + boss_.body().w, 3);
	}
	const SDL_Rect beam = toRect(boss_.beam());
	if (boss_.beamActive() && SDL_HasIntersection(&pr, &beam)) hurtPlayer(HurtSound::Electric);
}

void World::physics() {
	if (isOver_) return;
	const float now = time();
	const PlayerState& s = player_.state();
	if (course_.arena > 0 && !arena_ && s.x >= course_.arena + kArenaTrigger) enterArena();
	if (!arena_ && course_.arena <= 0 && s.x >= course_.finish) {
		end(potions_ >= cfg::kRequiredPotions ? Status::GoodEnd : Status::TrueEnd);
		return;
	}
	if (arena_ && boss_.phase() == GigaGolem::Phase::Dead) {
		end(potions_ >= cfg::kRequiredPotions ? Status::GoodEnd : Status::TrueEnd);
		return;
	}
	const bool caught = !arena_ && boss_.touches(player_) && !god_;
	if (s.lives <= 0 || s.y > cfg::kWindowH + 250 || caught) {
		if (difficulty_ != cfg::kDifficultyHard && (arena_ || hasCheckpoint_)) respawn();
		else end(Status::GameOver);
		return;
	}
	if (s.isImmortal) player_.updateImmortal(now);
	player_.integrate(assets_.jumpSound());
	if (arena_) {
		player_.confine(course_.arena, course_.arena + cfg::kWindowW);
		scrollX_ += (-course_.arena - scrollX_) * 0.1f;  // slide to the locked camera
	}
	else {
		scrollX_ = SDL_min(0.0f, -player_.state().x + 350);
	}
	if (arena_ || now - bossStart_ >= tuning_.bossDelaySec) boss_.update(*this, cfg::kStep);
}

void World::enterArena() {
	arena_ = true;
	projectiles_.clear();
	boss_.startArena(course_.arena, static_cast<int>(tuning_.bossHp));
	shake(tuning_.shakeHit);
}

void World::respawn() {
	++respawns_;
	const float now = time();
	projectiles_.clear();
	if (arena_) {
		player_.respawn(course_.arena + 700, cfg::kGroundTop - 2.0f * cfg::kIdleH, difficulty_, now);
		boss_.startArena(course_.arena, static_cast<int>(tuning_.bossHp));
	}
	else {
		player_.respawn(cpX_, cpFeet_ - 2.0f * cfg::kIdleH, difficulty_, now);
		boss_.placeAt(SDL_max(0.0f, cpX_ - tuning_.respawnBossGap));
		bossStart_ = now;
		scrollX_ = SDL_min(0.0f, -cpX_ + 350);
	}
	shake(tuning_.shakeHit);
}

void World::end(Status s) {
	isOver_ = true;
	status_ = s;
	if (s == Status::GameOver) {
		Mix_HaltChannel(-1);
		Mix_HaltChannel(1);
	}
	Mix_HaltMusic();
	overTiming_ = time();
	endMs_ = static_cast<Uint32>(ticks_) * 1000 / 60;
	over_.timeMs = endMs_;
	over_.potions = potions_;
	over_.respawns = respawns_;
}

bool World::overReady() const { return isOver_ && time() - overTiming_ >= 2.0f; }

void World::debugWarp(bool toGoal) {
	if (isOver_) return;
	float x = 0, y = 100;
	if (toGoal) {
		if (arena_) return;
		x = course_.goal() - 400;
	}
	else {
		SDL_FPoint feet{};
		if (!level_.nextCheckpoint(player_.state().x + 60, feet)) return;
		x = feet.x;
		y = feet.y - 2.0f * cfg::kIdleH;
	}
	player_.respawn(x, y, player_.state().lives, time());
	boss_.placeAt(SDL_max(0.0f, x - tuning_.respawnBossGap));
	bossStart_ = time();
	projectiles_.clear();
	scrollX_ = SDL_min(0.0f, -x + 350);
}

float World::shakeOffset() {
	if (shake_ <= 0) return 0;
	seed_ = seed_ * 1664525u + 1013904223u;
	return (static_cast<float>(seed_ >> 8) / 8388608.0f - 1.0f) * shake_;
}

HudInfo World::hudInfo() const {
	HudInfo h;
	const PlayerState& s = player_.state();
	h.lives = s.lives;
	h.potions = potions_;
	h.key = keyObtained_;
	h.tenths = ticks_ / 6;
	const float goal = SDL_max(1.0f, course_.goal());
	h.progress = s.x / goal;
	h.bossProgress = boss_.x() / goal;
	const float camLeft = -scrollX_;
	h.bossWarn = !arena_ && boss_.x() + cfg::kGigaGolemSize < camLeft &&
		camLeft - (boss_.x() + cfg::kGigaGolemSize) < tuning_.bossWarnPx && ticks_ / 15 % 2 == 0;
	if (arena_ && boss_.fighting()) {
		h.bossHp = boss_.hp();
		h.bossMaxHp = boss_.maxHp();
	}
	h.god = god_;
	return h;
}

void World::renderOver(SDL_Renderer* r, const char* title, SDL_Color fg) {
	Mix_HaltChannel(-1);
	if (!Mix_PlayingMusic()) Mix_PlayMusic(assets_.endingMusic(static_cast<int>(status_)), -1);
	Parallax::drawStatic(r, assets_);
	hud_.drawOver(r, assets_, title, fg, overReady(), over_);
}

void World::render(SDL_Renderer* r, bool paused) {
	switch (status_) {
	case Status::GameOver: renderOver(r, "Bad End", SDL_Color{ 210, 8, 45, 255 }); break;
	case Status::TrueEnd: renderOver(r, "True End", SDL_Color{ 200, 112, 126, 255 }); break;
	case Status::GoodEnd: renderOver(r, "Good End", SDL_Color{ 96, 106, 116, 255 }); break;
	case Status::Playing: {
		const float sx = scrollX_ + (paused ? 0.0f : shakeOffset());
		parallax_.draw(r, assets_, sx);
		level_.drawSegments(r, assets_, sx);
		level_.drawGrounds(r, assets_, sx);
		for (const Projectile& p : projectiles_) {
			const int x = static_cast<int>(sx + p.box.x), y = static_cast<int>(p.box.y);
			if (p.kind == ProjectileKind::Arm) {  // glowing arm: 6 frames of 100x100, the arm at (58, 28) 40x18
				const int f = animFrame(6, 80);
				const SDL_Rect src{ f % 3 * 100 + 58, f / 3 * 100 + 28, 40, 18 }, d{ x - 8, y - 6, 80, 36 };
				const double angle = std::atan2(p.dy, p.dx) * 57.29578;
				SDL_RenderCopyEx(r, assets_.tex(TextureId::Arm), &src, &d, angle, nullptr, SDL_FLIP_NONE);
			}
			else {  // Arrow.png points left
				const SDL_Rect d{ x, y + 2, 58, 6 };
				SDL_RenderCopyEx(r, assets_.tex(TextureId::Arrow), nullptr, &d, 0, nullptr, p.dx > 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
			}
		}
		player_.draw(r, assets_, sx, time());
		boss_.draw(r, assets_, sx);
		particles_.draw(r, sx);
		hud_.drawStatus(r, assets_, hudInfo());
		if (!Mix_PlayingMusic()) Mix_PlayMusic(assets_.inGameMusic(music_), -1);
		break;
	}
	}
}
}
