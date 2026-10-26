#include "game/Level.h"
#include "game/EntityFactory.h"
#include "game/GameAssets.h"
#include "game/Player.h"
#include "game/World.h"
#include <algorithm>

namespace jr {
namespace {
constexpr int kCheckpointTile = 23;
}

Segment Level::makeSegment(const Grid& g, float x) const {
	Segment seg;
	for (int row = 0; row < cfg::kGridRows; ++row) {
		for (int col = 0; col < cfg::kGridCols; ++col) {
			if (!checkpoints_ && g[row][col] == kCheckpointTile) continue;
			if (auto e = makeEntity(g[row][col], x + col * cfg::kTile, static_cast<float>(row * cfg::kTile))) {
				seg.entities.push_back(std::move(e));
			}
		}
	}
	return seg;
}

void Level::build(const Course& c, bool checkpoints) {
	checkpoints_ = checkpoints;
	grounds_.clear();
	segments_.clear();
	for (float x : c.groundX) {
		grounds_.push_back({ x, static_cast<float>(cfg::kGroundTop), static_cast<float>(cfg::kGroundW),
			static_cast<float>(cfg::kGroundH) });
	}
	for (const auto& s : c.segments) segments_.push_back(makeSegment(c.grids[s.grid], s.x));
}

void Level::swapOnKey(const Course& c) {
	if (c.onKeySegment < 0) return;
	segments_[c.onKeySegment] = makeSegment(c.grids[c.onKeyGrid], c.segments[c.onKeySegment].x);
}

void Level::collideGround(Player& p, Mix_Chunk* landing) const {
	for (const SDL_FRect& g : grounds_) p.resolveAgainst(g, landing);  // no intersection gate, no clearHurt (as C)
}

void Level::collideSegments(Player& p, World& w, float dt) {
	for (auto& seg : segments_) {
		for (auto& e : seg.entities) {
			const SDL_FRect b = e->bounds();  // before the golem moves (C reads bx/by before updateGolem)
			const SDL_Rect pr = p.hitRect();
			const SDL_Rect er = toIntRect(b);
			e->update(p, w, dt);
			if (e->isDead()) continue;
			if (e->grantsKey()) w.setKeyObtained();  // checked before the hit test: takes effect next tick
			if (!SDL_HasIntersection(&pr, &er)) continue;
			e->onTouch(p, w);
			if (!e->isSolid()) {
				if (e->isLethal()) w.hurtPlayer(e->hurtSound());
			}
			else {
				p.clearHurt();
				p.resolveAgainst(b, w.assets().landing());
			}
		}
	}
}

void Level::hitEntities(const SDL_Rect& r, World& w) {
	for (auto& seg : segments_) {
		for (auto& e : seg.entities) {
			const SDL_Rect er = toIntRect(e->bounds());
			if (SDL_HasIntersection(&r, &er)) e->hit(w);
		}
	}
}

void Level::prune() {
	for (auto& seg : segments_) {
		auto& v = seg.entities;
		v.erase(std::remove_if(v.begin(), v.end(), [](const std::unique_ptr<Entity>& e) { return e->isGone(); }), v.end());
	}
}

bool Level::nextCheckpoint(float afterX, SDL_FPoint& feet) const {
	bool found = false;
	forEachEntity([&](const Entity& e) {
		const SDL_FRect& b = e.bounds();
		if (e.isCheckpoint() && b.x > afterX && (!found || b.x < feet.x)) {
			feet = { b.x, b.y + b.h };
			found = true;
		}
	});
	return found;
}

void Level::animate(Uint64 worldMs) {
	for (auto& seg : segments_)
		for (auto& e : seg.entities) e->animate(worldMs);
}

void Level::drawSegments(SDL_Renderer* r, const GameAssets& a, float scrollX) const {
	forEachEntity([&](const Entity& e) { e.draw(r, a, scrollX); });
}

void Level::drawGrounds(SDL_Renderer* r, const GameAssets& a, float scrollX) const {
	for (const SDL_FRect& g : grounds_) {
		const SDL_Rect src{ 217, 116, static_cast<int>(g.w), static_cast<int>(g.h) };
		const SDL_Rect d{ static_cast<int>(scrollX + g.x), static_cast<int>(g.y), static_cast<int>(g.w), static_cast<int>(g.h) };
		SDL_RenderCopyEx(r, a.tex(TextureId::Ground), &src, &d, 0, nullptr, SDL_FLIP_NONE);
	}
}
}
