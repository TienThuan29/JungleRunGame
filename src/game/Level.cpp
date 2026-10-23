#include "game/Level.h"
#include "game/EntityFactory.h"
#include "game/GameAssets.h"
#include "game/Player.h"
#include "game/World.h"

namespace jr {
Segment Level::makeSegment(const Grid& g, float x) {
	Segment seg;
	for (int row = 0; row < cfg::kGridRows; ++row) {
		for (int col = 0; col < cfg::kGridCols; ++col) {
			if (auto e = makeEntity(g[row][col], x + col * cfg::kTile, static_cast<float>(row * cfg::kTile))) {
				seg.entities.push_back(std::move(e));
			}
		}
	}
	return seg;
}

void Level::build(const Course& c) {
	grounds_.clear();
	segments_.clear();
	for (float x : c.groundX) {
		grounds_.push_back({ x, static_cast<float>(cfg::kWindowH - cfg::kGroundH), static_cast<float>(cfg::kGroundW),
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
			e->preCollide(p, dt);
			if (e->grantsKey()) w.setKeyObtained();  // checked before the hit test: takes effect next frame
			if (!SDL_HasIntersection(&pr, &er)) continue;
			e->onTouch(p, w);
			if (!e->isSolid()) {
				if (e->isLethal() && !p.state().isImmortal) p.hurt(w.assets().hurt(e->hurtSound()));
			}
			else {
				p.clearHurt();
				p.resolveAgainst(b, w.assets().landing());
			}
		}
	}
}

void Level::animate(Uint64 nowMs) {
	for (auto& seg : segments_)
		for (auto& e : seg.entities) e->animate(nowMs);
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
