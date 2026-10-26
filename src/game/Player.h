#pragma once
#include "game/Config.h"
#include "game/Tuning.h"
#include <SDL_mixer.h>

namespace jr {
class GameAssets;

struct InputState {
	bool left = false, right = false, down = false;
};

struct PlayerState {
	float x = 500, y = 220, dx = 0, dy = 0;
	int lives = 0;
	int isTakenDamage = 0, isImmortal = 0;
	float immortalStartTime = 0, immortalSec = 1;
	int onLedge = 0, flip = 0;
	int status = 0;  // 0 idle, 1 run, 2 jump, 3 mid air, 4 falling
};

// Movement and collision response ported from the C version (running hitbox uses the idle size,
// onLedge is cleared only by jumping, ...). Changes: numbers come from Tuning, times are world
// seconds, and standing still keeps the facing direction (so the sword can hit things behind).
class Player {
public:
	void reset(int lives, const Tuning& t);
	void input(const InputState& in);                                // once per tick
	void jump();                                                    // only from a ledge
	void hitInfo(float& px, float& py, float& pw, float& ph) const;  // = load_player_info()
	SDL_Rect hitRect() const;
	void resolveAgainst(const SDL_FRect& b, Mix_Chunk* landing);     // = collision_correction()
	void hurt(Mix_Chunk* sound, float now);                          // caller checks immortality
	void clearHurt() { st_.isTakenDamage = 0; }
	void heal(int maxLives);
	void updateImmortal(float now);
	void integrate(Mix_Chunk* jumpSound);                            // move, gravity, airborne status
	void respawn(float x, float y, int lives, float now);            // standing, immortal for a while
	void knockback(float x, float dx) { st_.x = x; st_.dx = dx; }
	void confine(float minX, float maxX);                            // boss arena walls

	bool attack(float now, Mix_Chunk* swing);                        // false while cooling down
	bool attacking(float now) const { return now < attackUntil_; }
	SDL_Rect attackRect() const;
	int swing() const { return swing_; }                             // increments per attack

	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX, float now) const;
	const PlayerState& state() const { return st_; }
private:
	PlayerState st_;
	const Tuning* t_ = nullptr;
	float attackStart_ = -1, attackUntil_ = 0, nextAttack_ = 0;
	int swing_ = 0;
};
}
