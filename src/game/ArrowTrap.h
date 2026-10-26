#pragma once
#include "game/Entity.h"

namespace jr {
// Codes 24 (shoots left) and 25 (shoots right). Harmless itself; spawns an arrow projectile once per cycle.
// Sheet "Arrow Trap - Level 1": 17 frames of 96x32; the trap is the middle 32 px, the arrow leaves on frame 9.
class ArrowTrap final : public Entity {
public:
	ArrowTrap(SDL_FRect box, bool right) : Entity(box), right_(right) {}
	void update(const Player& p, World& w, float dt) override;
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const override;
private:
	static constexpr int kFrames = 17, kFireFrame = 9;
	bool right_;
	float t_ = 0;
	int frame_ = 0;
};
}
