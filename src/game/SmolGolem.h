#pragma once
#include "game/Entity.h"

namespace jr {
// Code 19: chases the player when near, always lethal.
class SmolGolem final : public Entity {
public:
	explicit SmolGolem(SDL_FRect box) : Entity(box) {}
	void preCollide(const Player& p, float dt) override;  // = detectPlayer + updateGolem
	bool isLethal() const override { return true; }
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const override;
private:
	float dx_ = 0;
	int flip_ = 0;
};
}
