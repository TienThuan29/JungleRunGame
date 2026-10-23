#include "core/Game.h"
#include "scenes/Scenes.h"

namespace jr {
namespace {
#ifdef _DEBUG
const char* name(SceneId id) {
	static const char* const names[] = { "QUIT", "MENU", "STORY", "HELP", "DONATE", "GAMEMODE", "INTRO", "GAMEPLAY" };
	return names[static_cast<int>(id)];
}
#endif
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
	click_ = sdl::loadChunk("Resource\\Sound\\Sound Effects\\click.wav");
	menuMusic_ = sdl::loadMusic("Resource\\Sound\\Soundtrack\\MainMenu2.mp3");
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

void Game::resize(int w, int h) {
	int cw = 0, ch = 0;
	SDL_GetWindowSize(window_.get(), &cw, &ch);
	if (cw == w && ch == h) return;
	SDL_SetWindowSize(window_.get(), w, h);
	SDL_SetWindowPosition(window_.get(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
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
