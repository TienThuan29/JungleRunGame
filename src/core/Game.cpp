#include "core/Game.h"
#include "core/SaveFile.h"
#include "scenes/Scenes.h"

namespace jr {
namespace {
#ifdef _DEBUG
const char* name(SceneId id) {
	static const char* const names[] = { "QUIT", "MENU", "STORY", "HELP", "DONATE", "GAMEMODE", "INTRO", "GAMEPLAY",
		"WORLDSELECT", "SETTINGS" };
	return names[static_cast<int>(id)];
}
#endif

SDL_Keycode padKey(Uint8 button) {
	switch (button) {
	case SDL_CONTROLLER_BUTTON_A: return SDLK_SPACE;
	case SDL_CONTROLLER_BUTTON_B: return SDLK_ESCAPE;
	case SDL_CONTROLLER_BUTTON_X: return SDLK_j;
	case SDL_CONTROLLER_BUTTON_Y: return SDLK_r;
	case SDL_CONTROLLER_BUTTON_BACK: return SDLK_x;
	case SDL_CONTROLLER_BUTTON_START: return SDLK_ESCAPE;
	case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return SDLK_n;
	case SDL_CONTROLLER_BUTTON_DPAD_UP: return SDLK_UP;
	case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return SDLK_DOWN;
	case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return SDLK_LEFT;
	case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return SDLK_RIGHT;
	default: return SDLK_UNKNOWN;
	}
}
}

Game::Game() {
	if (!session_.ok()) {
		fatal(SDL_GetError());
		return;
	}
	window_.reset(SDL_CreateWindow("Jungle Run", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, cfg::kMenuW, cfg::kMenuH, 0));
	if (!window_) {
		fatal(SDL_GetError());
		return;
	}
	renderer_.reset(SDL_CreateRenderer(window_.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
	if (!renderer_) {
		fatal(SDL_GetError());
		return;
	}
	SDL_SetRenderDrawBlendMode(renderer_.get(), SDL_BLENDMODE_BLEND);  // overlays, fades, particles
	SDL_RenderSetLogicalSize(renderer_.get(), cfg::kMenuW, cfg::kMenuH);
	click_ = sdl::loadChunk("Resource\\Sound\\Sound Effects\\click.wav");
	menuMusic_ = sdl::loadMusic("Resource\\Sound\\Soundtrack\\MainMenu2.mp3");
	loadSettings(settings_);
	applyAudio();
	if (settings_.fullscreen) setFullscreen(true);
	ok_ = true;
}

std::unique_ptr<Scene> Game::makeScene(SceneId id) {
	switch (id) {
	case SceneId::Menu: return std::make_unique<MenuScene>(*this);
	case SceneId::Story: return std::make_unique<StoryScene>(*this);
	case SceneId::Help: return std::make_unique<HelpScene>(*this);
	case SceneId::Donate: return std::make_unique<DonateScene>(*this);
	case SceneId::GameMode: return std::make_unique<GameModeScene>(*this);
	case SceneId::Intro: return std::make_unique<IntroScene>(*this);
	case SceneId::Gameplay: return std::make_unique<GameplayScene>(*this);
	case SceneId::WorldSelect: return std::make_unique<WorldSelectScene>(*this);
	case SceneId::Settings: return std::make_unique<SettingsScene>(*this);
	default: return nullptr;
	}
}

int Game::run() {
	SceneId id = SceneId::Menu;
	while (id != SceneId::Quit) {
		const SceneId prev = id;
		{
			auto scene = makeScene(id);
			id = scene ? scene->run() : SceneId::Quit;
		}  // the scene and everything it loaded is released here
#ifdef _DEBUG
		SDL_Log("[scene %s -> %s] %s", name(prev), name(id), sdl::liveCounts().c_str());
#else
		(void)prev;
#endif
	}
	return 0;
}

void Game::save() const { saveSettings(settings_); }

void Game::applyAudio() const {
	Mix_VolumeMusic(settings_.music);
	Mix_Volume(-1, settings_.sfx);  // multiplies each sound's own volume
}

void Game::setFullscreen(bool on) {
	settings_.fullscreen = on;
	SDL_SetWindowFullscreen(window_.get(), on ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	resize(logicalW_, logicalH_);
}

void Game::resize(int w, int h) {
	logicalW_ = w;
	logicalH_ = h;
	SDL_RenderSetLogicalSize(renderer_.get(), w, h);  // fullscreen: scaled with black bars
	if (settings_.fullscreen) return;
	int cw = 0, ch = 0;
	SDL_GetWindowSize(window_.get(), &cw, &ch);
	if (cw == w && ch == h) return;
	SDL_SetWindowSize(window_.get(), w, h);
	SDL_SetWindowPosition(window_.get(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

bool Game::handleGlobal(const SDL_Event& e) {
	switch (e.type) {
	case SDL_KEYDOWN:
		if (e.key.keysym.sym != SDLK_F11 || e.key.repeat) return false;
		setFullscreen(!settings_.fullscreen);
		save();
		return true;
	case SDL_CONTROLLERDEVICEADDED:
		if (!pad_) pad_.reset(SDL_GameControllerOpen(e.cdevice.which));
		return true;
	case SDL_CONTROLLERDEVICEREMOVED:
		if (pad_ && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pad_.get())) == e.cdevice.which) {
			pad_.reset();
			for (int i = 0; i < SDL_NumJoysticks() && !pad_; ++i) {  // switch to another connected pad
				if (SDL_IsGameController(i)) pad_.reset(SDL_GameControllerOpen(i));
			}
		}
		return true;
	case SDL_CONTROLLERBUTTONDOWN: {
		const SDL_Keycode k = padKey(e.cbutton.button);
		if (k == SDLK_UNKNOWN) return true;
		SDL_Event key{};
		key.type = SDL_KEYDOWN;
		key.key.timestamp = e.cbutton.timestamp;
		key.key.windowID = SDL_GetWindowID(window_.get());
		key.key.state = SDL_PRESSED;
		key.key.keysym.sym = k;
		key.key.keysym.scancode = SDL_GetScancodeFromKey(k);
		SDL_PushEvent(&key);
		return true;
	}
	default:
		return false;
	}
}

void Game::padInput(bool& left, bool& right, bool& down) const {
	left = right = down = false;
	if (!pad_) return;
	SDL_GameController* p = pad_.get();
	constexpr Sint16 dead = 12000;
	const Sint16 ax = SDL_GameControllerGetAxis(p, SDL_CONTROLLER_AXIS_LEFTX);
	const Sint16 ay = SDL_GameControllerGetAxis(p, SDL_CONTROLLER_AXIS_LEFTY);
	left = SDL_GameControllerGetButton(p, SDL_CONTROLLER_BUTTON_DPAD_LEFT) || ax < -dead;
	right = SDL_GameControllerGetButton(p, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) || ax > dead;
	down = SDL_GameControllerGetButton(p, SDL_CONTROLLER_BUTTON_DPAD_DOWN) || ay > dead;
}

void Game::tick() {
#ifdef _DEBUG
	const Uint32 now = SDL_GetTicks();
	if (now - lastTitle_ < 1000) return;
	lastTitle_ = now;
	SDL_SetWindowTitle(window_.get(), ("Jungle Run [" + sdl::liveCounts() + "]").c_str());
#endif
}

void Game::fatal(const std::string& msg) {
	SDL_Log("fatal: %s", msg.c_str());
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Jungle Run", msg.c_str(), window_.get());
}
}
