#include "scenes/Scenes.h"

namespace jr {
DonateScene::DonateScene(Game& g) : Scene(g, SceneId::Donate) {
	SDL_Renderer* r = g.renderer();
	bg_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\donate.bmp");
	qr_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\QR.bmp");
	logo_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\momologo.bmp");
	{
		const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
		back_ = std::make_unique<Button>(r, font.get(), "Return", SDL_Point{ 447, 440 }, color::black, color::aqua);
	}
	const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\Kanit-MediumItalic.ttf", 30);
	thanks_ = sdl::renderText(r, font.get(), "^^Thank You For Your Donation^^", SDL_Color{ 0, 255, 0, 255 });
	thankRect_ = sdl::sized(thanks_.get(), 247, 200);
	if (!bg_ || !back_->ok()) {
		g.fatal("Missing donate resources (see console)");
		go(SceneId::Quit);
	}
}

void DonateScene::handleEvent(const SDL_Event& e) {
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

void DonateScene::render(SDL_Renderer* r) {
	static constexpr SDL_Rect qr{ 398, 255, 170, 170 }, logo{ 328, 300, 60, 60 };
	SDL_RenderCopy(r, bg_.get(), nullptr, nullptr);
	SDL_RenderCopy(r, qr_.get(), nullptr, &qr);
	SDL_RenderCopy(r, logo_.get(), nullptr, &logo);
	SDL_RenderCopy(r, thanks_.get(), nullptr, &thankRect_);
	back_->draw(r);
}
}
