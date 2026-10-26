#pragma once
#include "game/Config.h"

namespace jr {
class GameAssets;
class Player;
class World;

enum class HurtSound { Damage, Spike, Electric };

// One non-empty cell of a level grid. Collision rules live in Level::collideSegments.
class Entity {
public:
	virtual ~Entity() = default;
	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	virtual void update(const Player&, World&, float) {}   // every tick, before its own hit test
	virtual void onTouch(Player&, World&) {}               // Pickup effects
	virtual bool hit(World&) { return false; }             // sword; true if it reacted
	virtual bool isSolid() const { return false; }
	virtual bool isLethal() const { return false; }
	virtual HurtSound hurtSound() const { return HurtSound::Damage; }
	virtual bool grantsKey() const { return false; }       // a Key that has been picked up
	virtual bool isTaken() const { return false; }         // a Pickup that has been picked up
	virtual bool isCheckpoint() const { return false; }
	virtual bool isDead() const { return false; }          // no collision any more (dying animation)
	virtual bool isGone() const { return false; }          // Level removes it
	virtual void animate(Uint64) {}                        // world ms; Trap updates its stored flags
	virtual void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const = 0;

	const SDL_FRect& bounds() const { return box_; }

protected:
	explicit Entity(SDL_FRect box) : box_(box) {}
	// Same truncation as drawTilesMap(): { scrollX + x, y, w, h } converted to int.
	SDL_Rect dst(float scrollX) const {
		return { static_cast<int>(scrollX + box_.x), static_cast<int>(box_.y), static_cast<int>(box_.w), static_cast<int>(box_.h) };
	}
	SDL_FRect box_;
};

// Same truncation as rectA/rectB in collision_detect_map().
inline SDL_Rect toIntRect(const SDL_FRect& b) {
	return { static_cast<int>(b.x), static_cast<int>(b.y), static_cast<int>(b.w), static_cast<int>(b.h) };
}
}
