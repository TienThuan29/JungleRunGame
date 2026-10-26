#include "scenes/Scenes.h"

namespace jr {
namespace {
const char* const kLevels = "Resource\\Levels";
const char* const kTuning = "Resource\\tuning.txt";
}

GameplayScene::GameplayScene(Game& g) : Scene(g, SceneId::Gameplay) {
	g.resize(cfg::kWindowW, cfg::kWindowH);
	Mix_HaltMusic();  // menu music
	if (!assets_.load(g.renderer())) {
		g.fatal("Missing game resources (see console)\nReturning to menu.");
		go(SceneId::Menu);
		return;
	}
	std::string error;
	if (!loadWorlds(kLevels, worlds_, error)) {
		g.fatal(error + "\nReturning to menu.");
		go(SceneId::Menu);
		return;
	}
	pauseTitle_ = sdl::renderText(g.renderer(), assets_.overFont(), "Paused", color::white);
	pauseHint_ = sdl::renderText(g.renderer(), assets_.hudFont(), "Esc continue     R restart     X menu", SDL_Color{ 255, 255, 73, 255 });
	reloadTuning();
	loadWorld(SDL_clamp(g.settings().world, 0, static_cast<int>(worlds_.size()) - 1));
}

GameplayScene::~GameplayScene() {  // gameplay.c:90-91
	Mix_HaltChannel(-1);
	Mix_HaltMusic();
}

bool GameplayScene::loadWorld(int index) {
	world_.reset();  // it refers to course_
	worldIndex_ = index;
	game_.settings().world = index;
	std::string error;
	if (!loadCourse(kLevels, worlds_[index].course, course_, error)) {
		game_.fatal(error + "\nReturning to menu.");
		go(SceneId::Menu);
		return false;
	}
	world_ = std::make_unique<World>(assets_, course_, tuning_, game_.settings().difficulty, worlds_[index].music);
	restart();
	return true;
}

void GameplayScene::reloadTuning() {
	std::string problems;
	loadTuning(kTuning, tuning_, problems);
	if (!problems.empty()) {
		SDL_Log("tuning: %s", problems.c_str());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Jungle Run - Resource\\tuning.txt",
			(problems + "\nThese lines use the default value.").c_str(), nullptr);
	}
}

void GameplayScene::restart() {
	Mix_HaltMusic();
	world_->reset();
	paused_ = recorded_ = false;
}

bool GameplayScene::testKeys() const {
#ifdef _DEBUG
	return true;
#else
	return tuning_.debug > 0;
#endif
}

// Once per run: best time and unlocking (not for Bad End), then the ending screen can show them.
void GameplayScene::record() {
	recorded_ = true;
	Settings& s = game_.settings();
	Uint32& best = s.best[worldIndex_][s.modeIndex];
	const bool hasNext = worldIndex_ + 1 < static_cast<int>(worlds_.size());
	if (world_->status() == Status::GameOver) {
		world_->setRecord(best, false, false);
		return;
	}
	const Uint32 t = world_->timeMs();
	const bool isNew = best == 0 || t < best;
	if (isNew) best = t;
	if (hasNext) s.unlocked = SDL_max(s.unlocked, worldIndex_ + 2);
	game_.save();
	world_->setRecord(best, isNew, hasNext);
}

SceneId GameplayScene::run() {
	SDL_Renderer* r = game_.renderer();
	const Uint64 start = SDL_GetTicks64();
	Uint64 last = start;
	float acc = 0;
	while (running()) {
		const Uint64 now = SDL_GetTicks64();
		acc += static_cast<float>(now - last) / 1000.0f;
		last = now;
		SDL_Event e;
		while (running() && SDL_PollEvent(&e)) {
			if (game_.handleGlobal(e)) continue;
			if (e.type == SDL_QUIT) {
				go(SceneId::Quit);
				continue;
			}
			if (e.type != SDL_KEYDOWN) continue;
			const bool over = world_->overReady();
			const SDL_Keycode sym = e.key.keysym.sym;
			if (e.key.repeat && sym != SDLK_SPACE && sym != SDLK_UP && sym != SDLK_w) continue;  // holding Esc/R/F5 must not repeat
			switch (sym) {
			case SDLK_ESCAPE:
				if (world_->isOver()) break;
				paused_ = !paused_;
				if (paused_) Mix_PauseMusic();
				else Mix_ResumeMusic();
				break;
			case SDLK_SPACE:
			case SDLK_UP:
			case SDLK_w:
				if (!paused_) world_->jump();
				break;
			case SDLK_j:
			case SDLK_k:
			case SDLK_f:
			case SDLK_LCTRL:
				if (!paused_) world_->attack();
				break;
			case SDLK_r:
				if (paused_ || over) {
					Mix_ResumeMusic();
					reloadTuning();
					restart();
				}
				break;
			case SDLK_x:
				if (paused_ || over) go(SceneId::Menu);
				break;
			case SDLK_n:
				if (over && worldIndex_ + 1 < game_.settings().unlocked && worldIndex_ + 1 < static_cast<int>(worlds_.size()) &&
					world_->status() != Status::GameOver) {
					reloadTuning();
					loadWorld(worldIndex_ + 1);
				}
				break;
			case SDLK_F1:
				if (testKeys()) world_->toggleGod();
				break;
			case SDLK_F2:
			case SDLK_F3:
				if (testKeys() && !paused_) world_->debugWarp(sym == SDLK_F3);
				break;
			case SDLK_F5:
				if (testKeys()) reloadTuning();
				break;
			default:
				break;
			}
		}
		if (!running()) break;
		if (paused_) {
			acc = 0;
		}
		else {
			acc = SDL_min(acc, cfg::kMaxTicksPerFrame * cfg::kStep);  // a stalled window must not fast-forward
			const Uint8* k = SDL_GetKeyboardState(nullptr);
			bool padL = false, padR = false, padD = false;
			game_.padInput(padL, padR, padD);
			const InputState in{ k[SDL_SCANCODE_LEFT] || k[SDL_SCANCODE_A] || padL, k[SDL_SCANCODE_RIGHT] || k[SDL_SCANCODE_D] || padR,
				k[SDL_SCANCODE_DOWN] || k[SDL_SCANCODE_S] || padD };
			while (acc >= cfg::kStep) {
				world_->tick(in);
				acc -= cfg::kStep;
			}
		}
		if (world_->isOver() && !recorded_) record();
		SDL_RenderClear(r);
		world_->render(r, paused_);
		if (paused_) {
			SDL_SetRenderDrawColor(r, 0, 0, 0, 150);
			SDL_RenderFillRect(r, nullptr);
			SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
			SDL_Rect t = sdl::sized(pauseTitle_.get(), 0, 250);
			t.x = (cfg::kWindowW - t.w) / 2;
			SDL_RenderCopy(r, pauseTitle_.get(), nullptr, &t);
			SDL_Rect h = sdl::sized(pauseHint_.get(), 0, 420);
			h.x = (cfg::kWindowW - h.w) / 2;
			SDL_RenderCopy(r, pauseHint_.get(), nullptr, &h);
		}
		drawFade(r, start, 0.3f);
		SDL_RenderPresent(r);
		game_.tick();
	}
	return next_;
}
}
