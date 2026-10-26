#include "scenes/Scenes.h"

namespace jr {
namespace {
constexpr SDL_Rect kRight{ 714, 481, 73, 55 }, kLeft{ 206, 480, 74, 55 }, kScreen{ -70, 0, 1110, 668 };
bool inside(const SDL_Rect& r, int x, int y) { return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h; }
}

StoryScene::StoryScene(Game& g) : Scene(g, SceneId::Story) {
	SDL_Renderer* r = g.renderer();
	page_[0] = sdl::loadTexture(r, "Resource\\Menu\\Story\\story1.bmp");
	page_[1] = sdl::loadTexture(r, "Resource\\Menu\\Story\\story2.bmp");
	red_ = sdl::loadTexture(r, "Resource\\Menu\\Story\\red.bmp");
	green_ = sdl::loadTexture(r, "Resource\\Menu\\Story\\green.bmp");
	const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
	back_ = std::make_unique<Button>(r, font.get(), "Return", SDL_Point{ 450, 503 }, color::black, color::aqua);
	if (!page_[0] || !page_[1] || !red_ || !green_ || !back_->ok()) {
		g.fatal("Missing story resources (see console)");
		go(SceneId::Quit);
	}
}

void StoryScene::handleEvent(const SDL_Event& e) {
	if (e.type == SDL_MOUSEMOTION) {
		hotRight_ = inside(kRight, e.motion.x, e.motion.y);
		hotLeft_ = inside(kLeft, e.motion.x, e.motion.y);
		back_->hover(e.motion.x, e.motion.y);
	}
	else if (e.type == SDL_MOUSEBUTTONDOWN) {
		const int x = e.button.x, y = e.button.y;
		if (inside(kRight, x, y)) {
			Mix_PlayChannel(-1, game_.click(), 0);
			current_ = 1;
		}
		if (inside(kLeft, x, y)) {
			Mix_PlayChannel(-1, game_.click(), 0);
			current_ = 0;
		}
		if (back_->hit(x, y)) {
			Mix_PlayChannel(-1, game_.click(), 0);
			go(SceneId::Menu);
		}
	}
	else if (isKey(e, SDLK_RIGHT) || isKey(e, SDLK_d)) {
		current_ = 1;
	}
	else if (isKey(e, SDLK_LEFT) || isKey(e, SDLK_a)) {
		current_ = 0;
	}
	else if (isKey(e, SDLK_ESCAPE) || isConfirm(e)) {
		go(SceneId::Menu);
	}
}

void StoryScene::render(SDL_Renderer* r) {
	SDL_RenderCopy(r, page_[current_].get(), nullptr, &kScreen);
	if (current_ == 0) {  // page 1: "next" arrow on the right
		SDL_RenderCopyEx(r, hotRight_ ? red_.get() : green_.get(), nullptr, &kRight, 0, nullptr,
			hotRight_ ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
	}
	else {  // page 2: "back" arrow on the left
		SDL_RenderCopyEx(r, hotLeft_ ? red_.get() : green_.get(), nullptr, &kLeft, 0, nullptr,
			hotLeft_ ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
	}
	back_->draw(r);
}
}
