#include "game/Pickup.h"
#include "game/GameAssets.h"
#include "game/World.h"

namespace jr {
void Pickup::onTouch(Player& p, World& w) {
	if (taken_) return;
	taken_ = true;
	SDL_Color c{ 255, 230, 90, 255 };
	switch (kind_) {
	case PickupKind::FirstAid: p.heal(w.difficulty()); c = SDL_Color{ 255, 90, 120, 255 }; break;
	case PickupKind::Potion: w.addPotion(); break;
	case PickupKind::Key: break;  // grantsKey() reports it
	case PickupKind::Checkpoint: w.setCheckpoint(box_.x, box_.y + box_.h); c = SDL_Color{ 120, 255, 140, 255 }; break;
	}
	Mix_PlayChannel(-1, w.assets().itemPickUp(), 0);
	w.particles().burst(box_.x + box_.w / 2, box_.y + box_.h / 2, 12, c, 140);
}

void Pickup::draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const {
	if (taken_ && kind_ != PickupKind::Checkpoint) return;
	int frames = 8, row = 13;
	switch (kind_) {
	case PickupKind::FirstAid: frames = 7; row = 4; break;
	case PickupKind::Potion: row = 9; break;
	case PickupKind::Key: row = 13; break;
	case PickupKind::Checkpoint: row = 8; break;
	}
	SDL_Texture* t = a.tex(TextureId::Items);
	const SDL_Rect src{ animFrame(frames, cfg::kItemFrames) * 32, row * 32, 32, 32 };
	const SDL_Rect d = dst(scrollX);
	if (taken_) SDL_SetTextureColorMod(t, 120, 255, 140);
	SDL_RenderCopyEx(r, t, &src, &d, 0, nullptr, SDL_FLIP_NONE);
	if (taken_) SDL_SetTextureColorMod(t, 255, 255, 255);
}
}
