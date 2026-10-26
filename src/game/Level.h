#pragma once
#include "game/Entity.h"
#include "game/LevelLoader.h"
#include <SDL_mixer.h>
#include <memory>
#include <vector>

namespace jr {
class GameAssets;
class Player;
class World;

struct Segment {
	std::vector<std::unique_ptr<Entity>> entities;  // row-major, non-empty cells only
};

// Ground platforms + map segments of one run, in course order.
class Level {
public:
	void build(const Course& c, bool checkpoints);                  // from in-memory grids (also on Retry)
	void swapOnKey(const Course& c);                                // rebuild the onkey segment in place
	void collideGround(Player& p, Mix_Chunk* landing) const;        // = collision_detect_floor
	void collideSegments(Player& p, World& w, float dt);            // = collision_detect_map
	void hitEntities(const SDL_Rect& r, World& w);                  // sword
	void prune();                                                   // drop entities that are gone
	bool nextCheckpoint(float afterX, SDL_FPoint& feet) const;      // debug warp
	void animate(Uint64 worldMs);
	void drawSegments(SDL_Renderer* r, const GameAssets& a, float scrollX) const;
	void drawGrounds(SDL_Renderer* r, const GameAssets& a, float scrollX) const;  // = drawLedges
	template <class F>
	void forEachEntity(F&& f) const {
		for (const auto& s : segments_)
			for (const auto& e : s.entities) f(*e);
	}
private:
	Segment makeSegment(const Grid& g, float x) const;
	std::vector<SDL_FRect> grounds_;
	std::vector<Segment> segments_;
	bool checkpoints_ = true;  // false on Hard: code 23 is skipped
};
}
