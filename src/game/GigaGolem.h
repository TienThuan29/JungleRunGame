#pragma once
#include "game/GameAssets.h"
#include "game/Player.h"

namespace jr {
class World;

// The boss. Chase: follows the player from the left and throws its arm at the player now and then.
// Arena (course line `arena`): stands at the left wall and loops
//   Idle -> Charge (energy ball at the arm tip) -> Beam (chest high: stay on the ground) -> Open
//   Idle -> Throw (arm rolls along the floor: jump over it)                              -> Open
// Open = it can be hit. Sprite cells are 100x100 in Character_sheet.png; we draw the 76x50 crop
// at (24, 24) of a cell scaled 5x, so the golem faces right and its feet are 230 px below y.
class GigaGolem {
public:
	enum class Phase { Chase, Rise, Idle, Charge, Beam, Throw, Open, Dying, Dead };

	void reset() { placeAt(0); }
	void placeAt(float x);                     // chase from here (start, respawn, debug warp)
	void startArena(float arenaX, int hp);     // also restarts the fight after a respawn
	void update(World& w, float dt);
	bool touches(const Player& p) const;       // chase: caught = game over
	SDL_FRect body() const;                    // arena: contact hurts
	bool beamActive() const { return phase_ == Phase::Beam; }
	SDL_FRect beam() const;
	bool fighting() const { return phase_ != Phase::Chase && phase_ != Phase::Dying && phase_ != Phase::Dead; }
	bool hitBySword(World& w);                 // true if it took damage (only while Open)
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const;

	Phase phase() const { return phase_; }
	int hp() const { return hp_; }
	int maxHp() const { return maxHp_; }
	float x() const { return x_; }
	float right() const { return x_ + 0.75f * cfg::kGigaGolemSize; }  // chase: the edge that catches

private:
	static constexpr float kScale = 5, kW = 76 * kScale, kH = 50 * kScale, kFeet = 46 * kScale;
	static constexpr float kTipX = 68 * kScale, kTipY = 13 * kScale;  // arm tip, row 2 frames 5-7
	static constexpr float kBodyX = 30, kBodyW = 190;                 // torso inside the 250 px golem
	static constexpr float kRiseSec = 1.0f, kThrowSec = 0.6f, kThrowAt = 0.45f, kDyingSec = 1.2f;
	static constexpr float kWindupSec = 0.45f;                         // chase throw
	void enter(Phase p, float len) { phase_ = p; t_ = 0; len_ = len; }
	void drawCell(SDL_Renderer* r, SDL_Texture* t, int row, int frame, float scrollX) const;
	void drawCrumble(SDL_Renderer* r, SDL_Texture* t, int f, float scrollX) const;  // f 0..13
	int progress(int frames) const;  // frame of a one-shot animation over the current phase

	Phase phase_ = Phase::Chase;
	float x_ = 0, y_ = 0, dx_ = 0, t_ = 0, len_ = 1, arenaX_ = 0, fireT_ = 0, flash_ = 0;
	int hp_ = 0, maxHp_ = 1, cycle_ = 0;
	bool windup_ = false;
};
}
