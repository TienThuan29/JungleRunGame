#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <array>
#include <memory>
#include <string>

namespace jr::sdl {
enum class Kind { Texture, Font, Chunk, Music };
void track(Kind kind, int delta);  // Debug: live resource counts; Release: no-op
std::string liveCounts();          // Debug: "tex 42 font 3 wav 7 mus 2 | sdl_alloc 1234"; Release: ""

struct TextureDeleter { void operator()(SDL_Texture* p) const; };
struct FontDeleter { void operator()(TTF_Font* p) const; };
struct ChunkDeleter { void operator()(Mix_Chunk* p) const; };
struct MusicDeleter { void operator()(Mix_Music* p) const; };
struct WindowDeleter { void operator()(SDL_Window* p) const { SDL_DestroyWindow(p); } };
struct RendererDeleter { void operator()(SDL_Renderer* p) const { SDL_DestroyRenderer(p); } };

using TexturePtr = std::unique_ptr<SDL_Texture, TextureDeleter>;
using FontPtr = std::unique_ptr<TTF_Font, FontDeleter>;
using ChunkPtr = std::unique_ptr<Mix_Chunk, ChunkDeleter>;
using MusicPtr = std::unique_ptr<Mix_Music, MusicDeleter>;
using WindowPtr = std::unique_ptr<SDL_Window, WindowDeleter>;
using RendererPtr = std::unique_ptr<SDL_Renderer, RendererDeleter>;

TexturePtr loadTexture(SDL_Renderer* r, const std::string& path, bool keyBlack = false);
TexturePtr renderText(SDL_Renderer* r, TTF_Font* font, const std::string& text, SDL_Color color);
FontPtr loadFont(const std::string& path, int size);
ChunkPtr loadChunk(const std::string& path, int volume = -1);  // volume < 0: keep default
MusicPtr loadMusic(const std::string& path);
SDL_Rect sized(SDL_Texture* t, int x, int y);                // rect at (x, y) with the texture's size

template <size_t N>
void loadTextureSeq(SDL_Renderer* r, std::array<TexturePtr, N>& out, const char* fmt, bool keyBlack) {
	char path[256];
	for (size_t i = 0; i < N; ++i) {
		SDL_snprintf(path, sizeof path, fmt, static_cast<int>(i + 1));
		out[i] = loadTexture(r, path, keyBlack);
	}
}

template <size_t N>
bool allLoaded(const std::array<TexturePtr, N>& a) {
	for (const auto& t : a) {
		if (!t) return false;
	}
	return true;
}

// SDL + SDL_image/ttf/mixer init/quit.
class Session {
public:
	Session();
	~Session();
	Session(const Session&) = delete;
	Session& operator=(const Session&) = delete;
	bool ok() const { return ok_; }
private:
	bool ok_ = false;
};

// Mix_OpenAudio / Mix_CloseAudio. Failure is not fatal: the game runs silent.
class AudioDevice {
public:
	AudioDevice();
	~AudioDevice();
	AudioDevice(const AudioDevice&) = delete;
	AudioDevice& operator=(const AudioDevice&) = delete;
private:
	bool open_ = false;
};
}
