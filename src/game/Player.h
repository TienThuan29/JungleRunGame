#pragma once
#include "game/Config.h"
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
	float immortalStartTime = 0;
	int onLedge = 0, flip = 0;
	int status = 0;  // 0 idle, 1 run, 2 jump, 3 mid air, 4 falling
};

// Movement and collision response ported verbatim from the C version, quirks included
// (running hitbox uses the idle size, onLedge is cleared only by jumping, ...).
class Player {
public:
	void reset(int lives);
	void input(const InputState& in);                                // = movement part of handle_input()
	void jump();                                                    // only from a ledge
	void hitInfo(float& px, float& py, float& pw, float& ph) const;  // = load_player_info()
	SDL_Rect hitRect() const;
	void resolveAgainst(const SDL_FRect& b, Mix_Chunk* landing);     // = collision_correction()
	void hurt(Mix_Chunk* sound);                                     // caller checks lethal && !immortal
	void clearHurt() { st_.isTakenDamage = 0; }
	void heal(int maxLives);
	void updateImmortal(float nowSec);                               // = immortal_events()
	void integrate(Mix_Chunk* jumpSound);                            // move, gravity, airborne status
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const;
	void teleport(float x, float y) { st_.x = x; st_.y = y; }       // used by tools/parity only
	const PlayerState& state() const { return st_; }
private:
	PlayerState st_;
};
}
