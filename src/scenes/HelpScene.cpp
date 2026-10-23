#include "scenes/Scenes.h"

namespace jr {
HelpScene::HelpScene(Game& g) : Scene(g, SceneId::Help) {
	SDL_Renderer* r = g.renderer();
	bg_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\help.bmp");
	fire_ = sdl::loadTexture(r, "Resource\\Menu\\traps\\fire.bmp");
	saw_ = sdl::loadTexture(r, "Resource\\Menu\\traps\\saw.bmp");
	ground_ = sdl::loadTexture(r, "Resource\\Menu\\traps\\ground.bmp");
	lightning_ = sdl::loadTexture(r, "Resource\\Menu\\traps\\lightning.bmp");
	ceiling_ = sdl::loadTexture(r, "Resource\\Menu\\traps\\ceiling.bmp");
	sdl::loadTextureSeq(r, health_, "Resource\\Menu\\item\\health%d.bmp", true);
	sdl::loadTextureSeq(r, good_, "Resource\\Menu\\item\\good%d.bmp", true);
	sdl::loadTextureSeq(r, running_, "Resource\\Menu\\running\\run%d.bmp", false);
	sdl::loadTextureSeq(r, jumping_, "Resource\\Menu\\jump\\jump%d.bmp", false);
	const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
	back_ = std::make_unique<Button>(r, font.get(), "Return", SDL_Point{ 446, 503 }, color::black, color::aqua);
	if (!bg_ || !back_->ok()) {
		g.fatal("Missing help resources (see console)");
		go(SceneId::Quit);
	}
}

void HelpScene::handleEvent(const SDL_Event& e) {
	if (e.type == SDL_MOUSEMOTION) {
		back_->hover(e.motion.x, e.motion.y);
	}
	else if (e.type == SDL_MOUSEBUTTONDOWN && back_->hit(e.button.x, e.button.y)) {
		Mix_PlayChannel(-1, game_.click(), 0);
		go(SceneId::Menu);
	}
	else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
		go(SceneId::Menu);
	}
}

void HelpScene::render(SDL_Renderer* r) {
	static constexpr SDL_Rect health{ 550, 302, 40, 40 }, good{ 460, 365, 40, 40 }, running{ 500, 130, 35, 65 },
		jumping{ 430, 214, 35, 65 }, ceiling{ 610, 410, 60, 50 }, fire{ 670, 370, 55, 90 }, saw{ 720, 410, 50, 50 },
		ground{ 750, 410, 70, 50 }, lightning{ 793, 400, 70, 60 };
	const Uint64 t = SDL_GetTicks64();
	SDL_RenderCopy(r, bg_.get(), nullptr, nullptr);
	back_->draw(r);
	SDL_RenderCopy(r, lightning_.get(), nullptr, &lightning);
	SDL_RenderCopy(r, ground_.get(), nullptr, &ground);
	SDL_RenderCopy(r, fire_.get(), nullptr, &fire);
	SDL_RenderCopy(r, saw_.get(), nullptr, &saw);
	SDL_RenderCopy(r, health_[t / 120 % 7].get(), nullptr, &health);
	SDL_RenderCopy(r, good_[t / 120 % 8].get(), nullptr, &good);
	SDL_RenderCopy(r, running_[t / 120 % 8].get(), nullptr, &running);
	SDL_RenderCopy(r, jumping_[t / 200 % 5].get(), nullptr, &jumping);
	SDL_RenderCopy(r, ceiling_.get(), nullptr, &ceiling);
}
}
