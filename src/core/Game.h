#pragma once
#include "core/SdlHandles.h"
#include "game/Config.h"
#include <memory>
#include <string>

namespace jr {
enum class SceneId { Quit, Menu, Story, Help, Donate, GameMode, Intro, Gameplay };
class Scene;

struct Settings {
	int difficulty = cfg::kDifficultyEasy;
	int modeIndex = 0;  // 0 Easy, 1 Medium, 2 Hard (Game Mode label)
};

// Owns the SDL session, the single window/renderer, the audio device and menu sounds.
// Members are destroyed in reverse order: sounds -> audio -> renderer -> window -> SDL_Quit.
class Game {
public:
	Game();
	Game(const Game&) = delete;
	Game& operator=(const Game&) = delete;
	bool ok() const { return ok_; }
	int run();
	SDL_Renderer* renderer() const { return renderer_.get(); }
	Mix_Chunk* click() const { return click_.get(); }
	Mix_Music* menuMusic() const { return menuMusic_.get(); }
	Settings& settings() { return settings_; }
	void resize(int w, int h);
	void tick();                         // Debug: live counts in the window title once per second
	void fatal(const std::string& msg);  // log + message box
private:
	std::unique_ptr<Scene> makeScene(SceneId id);
	sdl::Session session_;
	sdl::WindowPtr window_;
	sdl::RendererPtr renderer_;
	sdl::AudioDevice audio_;
	sdl::ChunkPtr click_;
	sdl::MusicPtr menuMusic_;
	Settings settings_;
	bool ok_ = false;
	Uint32 lastTitle_ = 0;
};
}
