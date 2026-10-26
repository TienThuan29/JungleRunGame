#include "scenes/Scenes.h"

namespace jr {
namespace {
const char* const kModes[3] = { "Easy", "Medium", "Hard" };
const char* const kNotes[3] = { "5 lives, checkpoints", "3 lives, checkpoints", "1 life, no checkpoints" };
constexpr SDL_Color kModeColor[3] = { { 0, 255, 0, 255 }, { 255, 200, 0, 255 }, { 255, 0, 0, 255 } };
constexpr int kModeX[3] = { 435, 415, 435 };
}

GameModeScene::GameModeScene(Game& g) : Scene(g, SceneId::GameMode) {
	SDL_Renderer* r = g.renderer();
	bg_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\gamemode.bmp");
	trick_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\trick3.bmp");
	{
		const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
		back_ = std::make_unique<Button>(r, font.get(), "Return", SDL_Point{ 446, 439 }, color::black, color::aqua);
	}
	{
		const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\Kanit-Medium.ttf", 60);
		left_ = std::make_unique<Button>(r, font.get(), "<<", SDL_Point{ 310, 290 }, color::black, color::red);
		right_ = std::make_unique<Button>(r, font.get(), ">>", SDL_Point{ 595, 290 }, color::black, color::red);
	}
	{
		const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\crazy-pixel.ttf", 100);
		title_ = sdl::renderText(r, font.get(), "Game Mode", SDL_Color{ 0, 200, 255, 255 });
		titleRect_ = sdl::sized(title_.get(), 350, 130);
	}
	modeFont_ = sdl::loadFont("Resource\\Fonts\\crazy-pixel.ttf", 80);
	refreshMode();
	if (!bg_ || !back_->ok() || !modeFont_) {
		g.fatal("Missing game mode resources (see console)");
		go(SceneId::Quit);
	}
}

void GameModeScene::refreshMode() {
	const int i = game_.settings().modeIndex;
	mode_ = sdl::renderText(game_.renderer(), modeFont_.get(), kModes[i], kModeColor[i]);
	const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\Kanit-Light.ttf", 20);
	note_ = sdl::renderText(game_.renderer(), font.get(), kNotes[i], color::black);
}

void GameModeScene::step(int delta) {  // delta 1 = next, 2 = previous (mod 3)
	Settings& s = game_.settings();
	s.modeIndex = (s.modeIndex + delta) % 3;
	s.difficulty = cfg::kLives[s.modeIndex];
	refreshMode();
	game_.save();
}

void GameModeScene::handleEvent(const SDL_Event& e) {
	if (e.type == SDL_MOUSEMOTION) {
		back_->hover(e.motion.x, e.motion.y);
		left_->hover(e.motion.x, e.motion.y);
		right_->hover(e.motion.x, e.motion.y);
	}
	else if (e.type == SDL_MOUSEBUTTONDOWN) {
		const int x = e.button.x, y = e.button.y;
		if (back_->hit(x, y)) {
			Mix_PlayChannel(-1, game_.click(), 0);
			go(SceneId::Menu);
		}
		const int delta = left_->hit(x, y) ? 2 : right_->hit(x, y) ? 1 : 0;
		if (delta) {
			Mix_PlayChannel(-1, game_.click(), 0);
			step(delta);
		}
	}
	else if (isKey(e, SDLK_LEFT) || isKey(e, SDLK_a)) {
		Mix_PlayChannel(-1, game_.click(), 0);
		step(2);
	}
	else if (isKey(e, SDLK_RIGHT) || isKey(e, SDLK_d)) {
		Mix_PlayChannel(-1, game_.click(), 0);
		step(1);
	}
	else if (isKey(e, SDLK_ESCAPE) || isConfirm(e)) {
		go(SceneId::Menu);
	}
}

void GameModeScene::render(SDL_Renderer* r) {
	static constexpr SDL_Rect trick{ 390, 300, 190, 70 };
	const SDL_Rect modeRect = sdl::sized(mode_.get(), kModeX[game_.settings().modeIndex], 265);
	SDL_Rect noteRect = sdl::sized(note_.get(), 0, 392);
	noteRect.x = 482 - noteRect.w / 2;
	SDL_RenderCopy(r, bg_.get(), nullptr, nullptr);
	SDL_RenderCopy(r, trick_.get(), nullptr, &trick);
	back_->draw(r);
	left_->draw(r);
	right_->draw(r);
	SDL_RenderCopy(r, title_.get(), nullptr, &titleRect_);
	SDL_RenderCopy(r, mode_.get(), nullptr, &modeRect);
	SDL_RenderCopy(r, note_.get(), nullptr, &noteRect);
}
}
