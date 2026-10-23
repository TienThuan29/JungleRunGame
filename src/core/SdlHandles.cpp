#include "core/SdlHandles.h"

namespace jr::sdl {
#ifdef _DEBUG
namespace {
std::array<int, 4> live{};
}
void track(Kind kind, int delta) { live[static_cast<size_t>(kind)] += delta; }
std::string liveCounts() {
	char buf[128];
	SDL_snprintf(buf, sizeof buf, "tex %d font %d wav %d mus %d | sdl_alloc %d",
		live[0], live[1], live[2], live[3], SDL_GetNumAllocations());
	return buf;
}
#else
void track(Kind, int) {}
std::string liveCounts() { return {}; }
#endif

void TextureDeleter::operator()(SDL_Texture* p) const { SDL_DestroyTexture(p); track(Kind::Texture, -1); }
void FontDeleter::operator()(TTF_Font* p) const { TTF_CloseFont(p); track(Kind::Font, -1); }
void ChunkDeleter::operator()(Mix_Chunk* p) const { Mix_FreeChunk(p); track(Kind::Chunk, -1); }
void MusicDeleter::operator()(Mix_Music* p) const { Mix_FreeMusic(p); track(Kind::Music, -1); }

namespace {
TexturePtr fromSurface(SDL_Renderer* r, SDL_Surface* s, const std::string& what, bool keyBlack) {
	if (!s) {
		SDL_Log("load fail: %s: %s", what.c_str(), SDL_GetError());
		return nullptr;
	}
	if (keyBlack) {
		SDL_SetColorKey(s, SDL_TRUE, SDL_MapRGB(s->format, 0, 0, 0));
	}
	SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
	SDL_FreeSurface(s);
	if (!t) {
		SDL_Log("texture fail: %s: %s", what.c_str(), SDL_GetError());
		return nullptr;
	}
	track(Kind::Texture, +1);
	return TexturePtr(t);
}
}

TexturePtr loadTexture(SDL_Renderer* r, const std::string& path, bool keyBlack) {
	return fromSurface(r, IMG_Load(path.c_str()), path, keyBlack);
}

TexturePtr renderText(SDL_Renderer* r, TTF_Font* font, const std::string& text, SDL_Color color) {
	return fromSurface(r, font ? TTF_RenderText_Solid(font, text.c_str(), color) : nullptr, text, false);
}

FontPtr loadFont(const std::string& path, int size) {
	TTF_Font* f = TTF_OpenFont(path.c_str(), size);
	if (!f) {
		SDL_Log("font fail: %s: %s", path.c_str(), SDL_GetError());
		return nullptr;
	}
	track(Kind::Font, +1);
	return FontPtr(f);
}

ChunkPtr loadChunk(const std::string& path, int volume) {
	Mix_Chunk* c = Mix_LoadWAV(path.c_str());
	if (!c) {
		SDL_Log("sound fail: %s: %s", path.c_str(), SDL_GetError());
		return nullptr;
	}
	if (volume >= 0) {
		Mix_VolumeChunk(c, volume);
	}
	track(Kind::Chunk, +1);
	return ChunkPtr(c);
}

MusicPtr loadMusic(const std::string& path) {
	Mix_Music* m = Mix_LoadMUS(path.c_str());
	if (!m) {
		SDL_Log("music fail: %s: %s", path.c_str(), SDL_GetError());
		return nullptr;
	}
	track(Kind::Music, +1);
	return MusicPtr(m);
}

SDL_Rect sized(SDL_Texture* t, int x, int y) {
	SDL_Rect r{ x, y, 0, 0 };
	SDL_QueryTexture(t, nullptr, nullptr, &r.w, &r.h);
	return r;
}

Session::Session() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
		SDL_Log("SDL_Init: %s", SDL_GetError());
		return;
	}
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	Mix_Init(MIX_INIT_MP3);
	ok_ = true;
}

Session::~Session() {
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

AudioDevice::AudioDevice() {
	open_ = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048) == 0;
	if (!open_) {
		SDL_Log("audio: %s", SDL_GetError());
	}
}

AudioDevice::~AudioDevice() {
	if (open_) {
		Mix_HaltMusic();
		Mix_CloseAudio();
	}
}
}
