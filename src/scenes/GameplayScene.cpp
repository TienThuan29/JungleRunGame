#include "scenes/Scenes.h"

namespace jr {
GameplayScene::GameplayScene(Game& g) : Scene(g, SceneId::Gameplay) {
	g.resize(cfg::kWindowW, cfg::kWindowH);
	if (!assets_.load(g.renderer())) {
		g.fatal("Missing game resources (see console)\nReturning to menu.");
		go(SceneId::Menu);
		return;
	}
	std::string error;
	if (!loadCourse("Resource\\Levels", course_, error)) {
		g.fatal(error + "\nReturning to menu.");
		go(SceneId::Menu);
		return;
	}
	world_ = std::make_unique<World>(assets_, course_, g.settings().difficulty);
	world_->reset();
}

GameplayScene::~GameplayScene() {  // gameplay.c:90-91
	Mix_HaltChannel(-1);
	Mix_HaltMusic();
}

SceneId GameplayScene::run() {
	Uint64 last = SDL_GetTicks64();
	while (running()) {
		const Uint64 now = SDL_GetTicks64();
		const float dt = static_cast<float>(now - last) / 1000.0f;
		last = now;
		const Uint8* k = SDL_GetKeyboardState(nullptr);  // before polling, as in C
		world_->input({ k[SDL_SCANCODE_LEFT] || k[SDL_SCANCODE_A], k[SDL_SCANCODE_RIGHT] || k[SDL_SCANCODE_D],
			k[SDL_SCANCODE_DOWN] || k[SDL_SCANCODE_S] });
		SDL_Event e;
		while (running() && SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				go(SceneId::Quit);
			}
			else if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
				case SDLK_SPACE:
				case SDLK_UP:
				case SDLK_w:
					world_->jump();
					break;
				case SDLK_r:
					if (world_->overReady()) {
						Mix_HaltMusic();
						world_->reset();
					}
					break;
				case SDLK_x:
					if (world_->overReady()) go(SceneId::Menu);
					break;
				default:
					break;
				}
			}
		}
		if (!running()) break;
		world_->step(dt);
		world_->render(game_.renderer());
		game_.tick();
	}
	return next_;
}
}
