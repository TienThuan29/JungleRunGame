#include "game/Pickup.h"
#include "game/GameAssets.h"
#include "game/World.h"

namespace jr {
void Pickup::onTouch(Player& p, World& w) {
	if (taken_) return;
	taken_ = true;
	if (kind_ == PickupKind::FirstAid) p.heal(w.difficulty());
	else if (kind_ == PickupKind::Potion) w.addPotion();
	Mix_PlayChannel(-1, w.assets().itemPickUp(), 0);
}

void Pickup::draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const {
	if (taken_) return;
	const int frames = kind_ == PickupKind::FirstAid ? 7 : 8;
	const int row = kind_ == PickupKind::FirstAid ? 4 : kind_ == PickupKind::Potion ? 9 : 13;
	const SDL_Rect src{ animFrame(frames, cfg::kItemFrames) * 32, row * 32, 32, 32 };
	const SDL_Rect d = dst(scrollX);
	SDL_RenderCopyEx(r, a.tex(TextureId::Items), &src, &d, 0, nullptr, SDL_FLIP_NONE);
}
}
