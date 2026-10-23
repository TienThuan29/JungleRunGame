#include "game/GameAssets.h"

namespace jr {
namespace {
std::string trap(const char* rest) { return std::string("Resource\\Foozle_2DTR0001_Pixel_Trap_Pack\\") + rest; }
}

bool GameAssets::load(SDL_Renderer* r) {
	sdl::loadTextureSeq(r, idle_, "Resource\\Character\\Idle\\Idle_%d.png", true);
	sdl::loadTextureSeq(r, run_, "Resource\\Character\\Run\\Run_%d.png", true);
	sdl::loadTextureSeq(r, jump_, "Resource\\Character\\Jump\\Jump_%d.png", true);
	sdl::loadTextureSeq(r, background_, "Resource\\Jungle Asset Pack\\parallax background\\plx-%d.png", false);
	auto set = [&](TextureId id, const std::string& path, bool keyBlack) {
		tex_[static_cast<size_t>(id)] = sdl::loadTexture(r, path, keyBlack);
	};
	set(TextureId::Tileset, "Resource\\Jungle Asset Pack\\jungle tileset\\jungle tileset.png", false);
	set(TextureId::Ground, "Resource\\Some Bullshit Platform\\Platform 3.png", true);
	set(TextureId::Fire1, trap("Fire Trap\\PNGs\\Fire Trap - Level 1.png"), true);
	set(TextureId::Fire2, trap("Fire Trap\\PNGs\\Fire Trap - Level 2.png"), true);
	set(TextureId::Fire3, trap("Fire Trap\\PNGs\\Fire Trap - Level 3.png"), true);
	set(TextureId::Ceiling, trap("Ceiling Trap\\PNGs\\Ceiling Trap - Level 1.png"), true);
	set(TextureId::Saw, trap("Saw Trap\\PNGs\\Saw Trap - Level 1.png"), true);
	set(TextureId::Lightning2, trap("Lightning Trap\\PNGs\\Lightning Trap - Level 2.png"), true);
	set(TextureId::Items, "Resource\\platformer items\\animated_items.png", true);
	set(TextureId::Golem, "Resource\\Mecha-stone Golem 0.1\\PNG sheet\\Character_sheet.png", true);
	set(TextureId::Sparkle, "Resource\\sparkle\\sparkle.png", true);

	hudFont_ = sdl::loadFont("Resource\\Fonts\\crazy-pixel.ttf", 48);
	overFont_ = sdl::loadFont("Resource\\Fonts\\Kanit-Light.ttf", 108);

	damage_ = sdl::loadChunk("Resource\\Sound\\Sound Effects\\DamageSound.wav", 64);
	electric_ = sdl::loadChunk("Resource\\Sound\\Sound Effects\\ElectricHurtSound.wav", 64);
	itemPickUp_ = sdl::loadChunk("Resource\\Sound\\Sound Effects\\ItemPickupSound.wav", 96);
	jumpSound_ = sdl::loadChunk("Resource\\Sound\\Sound Effects\\jump.wav", 16);
	landing_ = sdl::loadChunk("Resource\\Sound\\Sound Effects\\LandingSound.wav", 16);
	spike_ = sdl::loadChunk("Resource\\Sound\\Sound Effects\\SpikeSound.wav", 32);
	inGame_ = sdl::loadMusic("Resource\\Sound\\Soundtrack\\In Game 3.mp3");
	ending_[0] = sdl::loadMusic("Resource\\Sound\\Soundtrack\\Bad Ending.mp3");
	ending_[1] = sdl::loadMusic("Resource\\Sound\\Soundtrack\\True Ending.mp3");
	ending_[2] = sdl::loadMusic("Resource\\Sound\\Soundtrack\\Good Ending.mp3");
	Mix_VolumeMusic(32);

	return hudFont_ && overFont_ && sdl::allLoaded(tex_) && sdl::allLoaded(idle_) && sdl::allLoaded(run_) &&
		sdl::allLoaded(jump_) && sdl::allLoaded(background_);
}

Mix_Chunk* GameAssets::hurt(HurtSound s) const {
	switch (s) {
	case HurtSound::Spike: return spike_.get();
	case HurtSound::Electric: return electric_.get();
	default: return damage_.get();
	}
}
}
