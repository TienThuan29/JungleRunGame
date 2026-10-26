#pragma once
#include "game/GigaGolem.h"
#include "game/Hud.h"
#include "game/Level.h"
#include "game/Parallax.h"
#include "game/Particles.h"
#include "game/Player.h"
#include "game/Tuning.h"
#include <vector>

namespace jr {
enum class Status { GameOver = 0, TrueEnd = 1, GoodEnd = 2, Playing = 3 };  // 0-2 = ending music index

enum class ProjectileKind { Arm, Arrow };
struct Projectile {
	ProjectileKind kind;
	SDL_FRect box;
	float dx, dy, life;
};

// One run of the game: rules, player, boss, level. Runs in fixed ticks of cfg::kStep seconds;
// time() counts ticks, so pausing (not ticking) stops every gameplay timer.
class World {
public:
	World(const GameAssets& a, const Course& c, const Tuning& t, int difficulty, int music)
		: assets_(a), course_(c), tuning_(t), difficulty_(difficulty), music_(music) {}
	World(const World&) = delete;
	World& operator=(const World&) = delete;

	void reset();                                    // new run (also Retry)
	void tick(const InputState& in);                 // one fixed step
	void jump() { player_.jump(); }
	void attack();
	void render(SDL_Renderer* r);                    // no clear / present
	bool overReady() const;                          // ending screen shown >= 2 s

	// used by entities and the boss
	const GameAssets& assets() const { return assets_; }
	const Tuning& tuning() const { return tuning_; }
	int difficulty() const { return difficulty_; }
	float time() const { return static_cast<float>(ticks_) * cfg::kStep; }
	void addPotion() { ++potions_; }
	void setKeyObtained() { keyObtained_ = true; }
	void setCheckpoint(float x, float feetY);
	void hurtPlayer(HurtSound s);                    // no-op while invulnerable
	void spawnProjectile(ProjectileKind k, float x, float y, float dx, float dy);
	Particles& particles() { return particles_; }
	void shake(float amount) { shake_ = SDL_max(shake_, amount); }

	// read-only state
	const Player& player() const { return player_; }
	const Level& level() const { return level_; }
	int potions() const { return potions_; }
	bool keyObtained() const { return keyObtained_; }
	Status status() const { return status_; }
	bool isOver() const { return isOver_; }
	float scrollX() const { return scrollX_; }
	Uint32 timeMs() const { return endMs_; }

	// results (set by the scene once the run is over)
	void setRecord(Uint32 bestMs, bool newRecord, bool canNext) {
		over_.bestMs = bestMs;
		over_.newRecord = newRecord;
		over_.canNext = canNext;
	}

	// test keys
	void toggleGod() { god_ = !god_; }
	void debugWarp(bool toGoal);                     // next checkpoint, or just before the finish / arena

private:
	void physics();                                  // = add_physics + ending / respawn rules
	void combat();                                   // sword, projectiles, boss contact and laser
	void enterArena();
	void respawn();
	void end(Status s);
	void renderOver(SDL_Renderer* r, const char* title, SDL_Color fg);
	float shakeOffset();
	HudInfo hudInfo() const;

	const GameAssets& assets_;
	const Course& course_;
	const Tuning& tuning_;
	int difficulty_, music_;
	Player player_;
	GigaGolem boss_;
	Level level_;
	Parallax parallax_;
	Hud hud_;
	Particles particles_;
	std::vector<Projectile> projectiles_;
	Status status_ = Status::Playing;
	bool isOver_ = false, keyObtained_ = false, swapped_ = false, arena_ = false, god_ = false;
	bool hasCheckpoint_ = false;
	int potions_ = 0, respawns_ = 0, ticks_ = 0, bossHitSwing_ = 0;
	float overTiming_ = 0, bossStart_ = 0, scrollX_ = 0, cpX_ = 0, cpFeet_ = 0, shake_ = 0;
	Uint32 endMs_ = 0, seed_ = 1;
	OverInfo over_;
};
}
