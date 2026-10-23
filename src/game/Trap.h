#pragma once
#include "game/Entity.h"
#include "game/GameAssets.h"

namespace jr {
struct TrapSpec {
	TextureId tex;
	int frames, periodMs;    // frame i = nowMs / periodMs % frames
	SDL_Rect src;            // source rect of frame 0
	int stepX;               // frame i source x = src.x + i * stepX
	double angle;
	Uint32 lethalMask;       // bit i: frame i hurts
	Uint32 solidOnMask;      // bit i: frame i blocks
	Uint32 solidOffMask;     // bit i: frame i does not block; frames in neither mask keep the previous state
	HurtSound sound;
	int yRows, hRows;        // bounds: y += yRows * 50, h = hRows * 50
	bool initialLethal;      // codes 7 and 20 start lethal (loadMap)
};

// Lethal/solid are STORED and updated in animate() (render phase); collision reads the
// previous frame's values, like the C version (drawMap runs after collision).
class Trap final : public Entity {
public:
	Trap(SDL_FRect box, const TrapSpec& spec) : Entity(box), spec_(spec), lethal_(spec.initialLethal) {}
	bool isSolid() const override { return solid_; }
	bool isLethal() const override { return lethal_; }
	HurtSound hurtSound() const override { return spec_.sound; }
	void animate(Uint64 nowMs) override {
		frame_ = static_cast<int>(nowMs / static_cast<Uint64>(spec_.periodMs) % static_cast<Uint64>(spec_.frames));
		const Uint32 bit = 1u << frame_;
		lethal_ = (spec_.lethalMask & bit) != 0;
		if (spec_.solidOnMask & bit) solid_ = true;
		else if (spec_.solidOffMask & bit) solid_ = false;
	}
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const override {
		SDL_Rect src = spec_.src;
		src.x += frame_ * spec_.stepX;
		const SDL_Rect d = dst(scrollX);
		SDL_RenderCopyEx(r, a.tex(spec_.tex), &src, &d, spec_.angle, nullptr, SDL_FLIP_NONE);
	}
private:
	const TrapSpec& spec_;  // points into the static table in EntityFactory.cpp
	int frame_ = 0;
	bool lethal_;
	bool solid_ = false;
};
}
