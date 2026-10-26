#pragma once
#include "core/SdlHandles.h"
#include "game/Config.h"
#include <array>
#include <memory>
#include <string>

namespace jr {
enum class SceneId { Quit, Menu, Story, Help, Donate, GameMode, Intro, Gameplay, WorldSelect, Settings };
class Scene;

// Saved in save.txt next to the exe (see SaveFile.h), except `world`.
struct Settings {
	int modeIndex = 0;                     // 0 Easy, 1 Medium, 2 Hard (Game Mode)
	int difficulty = cfg::kDifficultyEasy; // = cfg::kLives[modeIndex], also max lives
	int music = 32, sfx = 128;             // 0-128 (SDL_mixer volume)
	bool fullscreen = false;
	bool introSeen = false;
	int unlocked = 1;                      // worlds 1..unlocked can be played
	std::array<std::array<Uint32, 3>, cfg::kMaxWorlds> best{};  // best time in ms per world and mode, 0 = none
	int world = 0;                         // chosen in World Select (not saved)
};

// Owns the SDL session, the single window/renderer, the audio device, the gamepad and menu sounds.
// Members are destroyed in reverse order: pad -> sounds -> audio -> renderer -> window -> SDL_Quit.
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
	void save() const;
	void applyAudio() const;
	void setFullscreen(bool on);
	void resize(int w, int h);
	// Every scene loop passes events here first. F11 toggles fullscreen; gamepad buttons come back
	// as key presses (A Space, B Esc, X J, Y R, Back X, Start Esc, RB N, D-pad arrows). True = consumed.
	bool handleGlobal(const SDL_Event& e);
	void padInput(bool& left, bool& right, bool& down) const;  // D-pad or left stick
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
	sdl::ControllerPtr pad_;
	Settings settings_;
	bool ok_ = false;
	Uint32 lastTitle_ = 0;
	int logicalW_ = cfg::kMenuW, logicalH_ = cfg::kMenuH;
};
}
