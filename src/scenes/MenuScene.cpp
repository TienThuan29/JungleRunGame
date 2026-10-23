#include "scenes/Scenes.h"

namespace jr {
namespace {
const char* const kLabels[6] = { "Single Player", "Story", "Game Mode", "Help", "Donate", "Exit" };
const SDL_Point kAt[6] = { { 400, 170 }, { 440, 225 }, { 410, 280 }, { 450, 335 }, { 430, 390 }, { 450, 445 } };
const SceneId kTarget[6] = { SceneId::Intro, SceneId::Story, SceneId::GameMode, SceneId::Help, SceneId::Donate, SceneId::Quit };
}

MenuScene::MenuScene(Game& g) : Scene(g, SceneId::Menu) {
	g.resize(cfg::kMenuW, cfg::kMenuH);
	if (!Mix_PlayingMusic()) {
		Mix_VolumeMusic(32);
		Mix_PlayMusic(g.menuMusic(), -1);
	}
	SDL_Renderer* r = g.renderer();
	bg_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\ChooseBG.bmp");
	const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\crazy-pixel.ttf", 50);
	items_.reserve(6);
	for (int i = 0; i < 6; ++i) items_.emplace_back(r, font.get(), kLabels[i], kAt[i], color::black, color::red);
	if (!bg_ || !items_[0].ok()) {
		g.fatal("Missing menu resources (see console)");
		go(SceneId::Quit);
	}
}

MenuScene::~MenuScene() {
	if (next_ == SceneId::Intro) Mix_HaltMusic();  // menu.c:66
}

void MenuScene::handleEvent(const SDL_Event& e) {
	if (e.type == SDL_MOUSEMOTION) {
		for (Button& b : items_) b.hover(e.motion.x, e.motion.y);
	}
	else if (e.type == SDL_MOUSEBUTTONDOWN) {
		for (int i = 0; i < 6; ++i) {
			if (items_[i].hit(e.button.x, e.button.y)) {
				Mix_PlayChannel(-1, game_.click(), 0);
				go(kTarget[i]);
			}
		}
	}
	else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
		go(SceneId::Quit);
	}
}

void MenuScene::render(SDL_Renderer* r) {
	SDL_RenderCopy(r, bg_.get(), nullptr, nullptr);
	for (const Button& b : items_) b.draw(r);
}
}
