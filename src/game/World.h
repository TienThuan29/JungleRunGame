#pragma once
#include "game/GigaGolem.h"
#include "game/Hud.h"
#include "game/Level.h"
#include "game/Parallax.h"
#include "game/Player.h"

namespace jr {
enum class Status { GameOver = 0, TrueEnd = 1, GoodEnd = 2, Playing = 3 };  // 0-2 = ending music index

// One run of the game: rules, player, boss, level. GameplayScene only feeds input and asks it to draw.
class World {
public:
	World(const GameAssets& a, const Course& c, int difficulty) : assets_(a), course_(c), difficulty_(difficulty) {}
	World(const World&) = delete;
	World& operator=(const World&) = delete;

	void reset();                                    // = game_reset (new run, Retry)
	void input(const InputState& in) { player_.input(in); }
	void jump() { player_.jump(); }
	void step(float dt);                             // floor collision, segment collision, physics
	void render(SDL_Renderer* r);                    // = do_render (clear ... present)
	bool overReady() const;                          // ending screen shown >= 2 s

	// used by entities
	const GameAssets& assets() const { return assets_; }
	int difficulty() const { return difficulty_; }
	void addPotion() { ++potions_; }
	void setKeyObtained() { keyObtained_ = true; }

	// read-only state (HUD, tools/parity)
	const Player& player() const { return player_; }
	Player& player() { return player_; }
	const Level& level() const { return level_; }
	int potions() const { return potions_; }
	bool keyObtained() const { return keyObtained_; }
	Status status() const { return status_; }
	bool isOver() const { return isOver_; }
	float bossX() const { return boss_.x(); }
	float scrollX() const { return scrollX_; }

private:
	void physics(float dt);                          // = add_physics
	void renderOver(SDL_Renderer* r, const char* title, SDL_Color fg);

	const GameAssets& assets_;
	const Course& course_;
	int difficulty_;
	Player player_;
	GigaGolem boss_;
	Level level_;
	Parallax parallax_;
	Hud hud_;
	Status status_ = Status::Playing;
	bool isOver_ = false, keyObtained_ = false, swapped_ = false;
	int potions_ = 0;
	float overTiming_ = 0, bossStart_ = 0, scrollX_ = 0;
};
}
