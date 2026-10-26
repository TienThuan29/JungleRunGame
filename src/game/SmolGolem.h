#pragma once
#include "game/Entity.h"

namespace jr {
// Codes 19 (normal) and 26 (fast, orange): chases the player when near, lethal until cut by the sword.
class SmolGolem final : public Entity {
public:
	SmolGolem(SDL_FRect box, bool fast) : Entity(box), fast_(fast) {}
	void update(const Player& p, World& w, float dt) override;  // = detectPlayer + updateGolem
	bool hit(World& w) override;
	bool isLethal() const override { return !dying_; }
	bool isDead() const override { return dying_; }
	bool isGone() const override { return dying_ && deathT_ >= kDeathSec; }
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const override;
private:
	static constexpr float kDeathSec = 0.7f;
	float dx_ = 0, deathT_ = 0;
	int flip_ = 0;
	bool fast_, dying_ = false;
};
}
