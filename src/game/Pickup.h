#pragma once
#include "game/Entity.h"

namespace jr {
enum class PickupKind { FirstAid, Potion, Key };

// Codes 13, 14, 21: taken once, then hidden.
class Pickup final : public Entity {
public:
	Pickup(SDL_FRect box, PickupKind kind) : Entity(box), kind_(kind) {}
	void onTouch(Player& p, World& w) override;
	bool grantsKey() const override { return kind_ == PickupKind::Key && taken_; }
	bool isTaken() const override { return taken_; }
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const override;
private:
	PickupKind kind_;
	bool taken_ = false;
};
}
