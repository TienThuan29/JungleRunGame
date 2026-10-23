#pragma once
#include "core/SdlHandles.h"
#include "game/Entity.h"

namespace jr {
enum class TextureId { Tileset, Ground, Fire1, Fire2, Fire3, Ceiling, Saw, Lightning2, Items, Golem, Sparkle, Count };

// Owns every gameplay texture, font and sound. Entities hold raw pointers borrowed from here,
// so GameplayScene declares GameAssets before World.
class GameAssets {
public:
	bool load(SDL_Renderer* r);  // false if an image or font is missing; sounds are optional
	SDL_Texture* tex(TextureId id) const { return tex_[static_cast<size_t>(id)].get(); }
	SDL_Texture* idle(int i) const { return idle_[i].get(); }
	SDL_Texture* run(int i) const { return run_[i].get(); }
	SDL_Texture* jump(int i) const { return jump_[i].get(); }
	SDL_Texture* background(int i) const { return background_[i].get(); }
	TTF_Font* hudFont() const { return hudFont_.get(); }
	TTF_Font* overFont() const { return overFont_.get(); }
	Mix_Chunk* hurt(HurtSound s) const;
	Mix_Chunk* itemPickUp() const { return itemPickUp_.get(); }
	Mix_Chunk* landing() const { return landing_.get(); }
	Mix_Chunk* jumpSound() const { return jumpSound_.get(); }
	Mix_Music* inGameMusic() const { return inGame_.get(); }
	Mix_Music* endingMusic(int i) const { return ending_[i].get(); }
private:
	std::array<sdl::TexturePtr, static_cast<size_t>(TextureId::Count)> tex_;
	std::array<sdl::TexturePtr, 12> idle_;
	std::array<sdl::TexturePtr, 8> run_;
	std::array<sdl::TexturePtr, 4> jump_;
	std::array<sdl::TexturePtr, 5> background_;
	sdl::FontPtr hudFont_, overFont_;
	sdl::ChunkPtr damage_, electric_, spike_, itemPickUp_, landing_, jumpSound_;
	sdl::MusicPtr inGame_;
	std::array<sdl::MusicPtr, 3> ending_;
};
}
