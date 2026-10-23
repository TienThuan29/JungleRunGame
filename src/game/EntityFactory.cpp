#include "game/EntityFactory.h"
#include "game/Decoration.h"
#include "game/Pickup.h"
#include "game/SmolGolem.h"
#include "game/Tile.h"
#include "game/Trap.h"
#include <initializer_list>

namespace jr {
namespace {
constexpr Uint32 bits(std::initializer_list<int> frames) {
	Uint32 m = 0;
	for (int f : frames) m |= 1u << f;
	return m;
}
constexpr Uint32 range(int first, int last) {
	Uint32 m = 0;
	for (int f = first; f <= last; ++f) m |= 1u << f;
	return m;
}
constexpr Uint32 all(int frames) { return range(0, frames - 1); }

struct TrapRow {
	int code;
	TrapSpec spec;
};

// Values from loadMap() + drawMap() in the C version (src/game/map.c).
const TrapRow kTraps[] = {
	{ 4, { TextureId::Fire1, 9, 200, { 8, 31, 16, 33 }, 32, 0, bits({ 4, 5 }), 0, 0, HurtSound::Damage, 0, 1, false } },
	{ 5, { TextureId::Fire2, 9, 175, { 3, 25, 26, 40 }, 32, 0, bits({ 4, 5 }), 0, 0, HurtSound::Damage, 0, 1, false } },
	{ 6, { TextureId::Fire3, 9, 250, { 3, 0, 26, 64 }, 32, 0, range(4, 8), 0, 0, HurtSound::Damage, -1, 2, false } },
	{ 7, { TextureId::Ceiling, 6, 250, { 16, 0, 33, 20 }, 0, 180, bits({ 0 }), 0, 0, HurtSound::Damage, 0, 1, true } },
	{ 8, { TextureId::Ceiling, 14, 250, { 15, 0, 34, 64 }, 64, 0, bits({ 1 }), range(2, 7), all(14) & ~range(1, 7), HurtSound::Spike, -2, 3, false } },
	{ 9, { TextureId::Ceiling, 14, 150, { 15, 0, 34, 64 }, 64, 0, bits({ 1 }), range(2, 7), all(14) & ~range(1, 7), HurtSound::Spike, -2, 3, false } },
	{ 10, { TextureId::Ceiling, 14, 75, { 15, 0, 34, 64 }, 64, 0, bits({ 1 }), range(2, 7), all(14) & ~range(1, 7), HurtSound::Spike, -2, 3, false } },
	{ 11, { TextureId::Saw, 16, 150, { 13, 0, 39, 35 }, 64, 180, all(16) & ~bits({ 0, 7, 14, 15 }), 0, 0, HurtSound::Damage, 0, 1, false } },
	{ 12, { TextureId::Saw, 16, 150, { 13, 0, 39, 35 }, 64, 0, all(16) & ~bits({ 0, 6, 7, 13, 14, 15 }), 0, 0, HurtSound::Damage, 0, 1, false } },
	{ 15, { TextureId::Lightning2, 12, 150, { 31, 0, 34, 96 }, 96, 0, range(3, 10), 0, 0, HurtSound::Electric, -2, 3, false } },
	{ 16, { TextureId::Lightning2, 12, 130, { 31, 0, 34, 96 }, 96, 0, range(3, 10), 0, 0, HurtSound::Electric, -2, 3, false } },
	{ 17, { TextureId::Lightning2, 12, 110, { 31, 0, 34, 96 }, 96, 0, range(3, 10), 0, 0, HurtSound::Electric, -2, 3, false } },
	{ 18, { TextureId::Lightning2, 12, 100, { 31, 0, 34, 96 }, 96, 0, range(3, 10), 0, 0, HurtSound::Electric, -2, 3, false } },
	{ 20, { TextureId::Ceiling, 6, 250, { 16, 0, 33, 20 }, 0, 0, bits({ 0 }), 0, 0, HurtSound::Damage, 0, 1, true } },
};
}

std::unique_ptr<Entity> makeEntity(int code, float x, float y) {
	const float t = static_cast<float>(cfg::kTile);
	const SDL_FRect cell{ x, y, t, t };
	switch (code) {
	case 0: return nullptr;
	case 1: return std::make_unique<Tile>(cell, SDL_Rect{ 63, 128, 50, 50 });
	case 2: return std::make_unique<Tile>(cell, SDL_Rect{ 463, 144, 50, 50 });
	case 3: return std::make_unique<Tile>(SDL_FRect{ x, y, 3 * t, t }, SDL_Rect{ 320, 144, 113, 49 });
	case 13: return std::make_unique<Pickup>(cell, PickupKind::FirstAid);
	case 14: return std::make_unique<Pickup>(cell, PickupKind::Potion);
	case 19: return std::make_unique<SmolGolem>(cell);
	case 21: return std::make_unique<Pickup>(cell, PickupKind::Key);
	case 22: return std::make_unique<Decoration>(cell);
	default: break;
	}
	for (const TrapRow& row : kTraps) {
		if (row.code == code) {
			const TrapSpec& s = row.spec;
			return std::make_unique<Trap>(SDL_FRect{ x, y + s.yRows * t, t, s.hRows * t }, s);
		}
	}
	return nullptr;  // unreachable: LevelLoader rejects codes outside 0-22
}
}
