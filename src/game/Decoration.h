#pragma once
#include "game/Entity.h"
#include "game/GameAssets.h"

namespace jr {
// Code 22: sparkle, draw only.
class Decoration final : public Entity {
public:
	explicit Decoration(SDL_FRect box) : Entity(box) {}
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const override {
		const int i = animFrame(16, cfg::kItemFrames);
		const SDL_Rect src{ i % 4 * 32, i / 4 * 32, 32, 32 };
		const SDL_Rect d = dst(scrollX);
		SDL_RenderCopyEx(r, a.tex(TextureId::Sparkle), &src, &d, 0, nullptr, SDL_FLIP_NONE);
	}
};
}
