#pragma once
#include "game/Entity.h"
#include "game/GameAssets.h"

namespace jr {
// Codes 1, 2, 3: always solid.
class Tile final : public Entity {
public:
	Tile(SDL_FRect box, SDL_Rect src) : Entity(box), src_(src) {}
	bool isSolid() const override { return true; }
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const override {
		const SDL_Rect d = dst(scrollX);
		SDL_RenderCopyEx(r, a.tex(TextureId::Tileset), &src_, &d, 0, nullptr, SDL_FLIP_NONE);
	}
private:
	SDL_Rect src_;
};
}
