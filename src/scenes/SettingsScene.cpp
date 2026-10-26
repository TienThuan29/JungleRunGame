#include "scenes/Scenes.h"

namespace jr {
namespace {
const char* const kLabels[3] = { "Music", "Sounds", "Fullscreen (F11)" };
constexpr int kRowY0 = 262, kRowDy = 58, kLabelX = 250, kLeftX = 545, kValueCx = 620, kRightX = 675, kStep = 16;
}

SettingsScene::SettingsScene(Game& g) : Scene(g, SceneId::Settings) {
	SDL_Renderer* r = g.renderer();
	bg_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\ChooseBG.bmp");
	{
		const sdl::FontPtr big = sdl::loadFont("Resource\\Fonts\\crazy-pixel.ttf", 60);
		title_ = sdl::renderText(r, big.get(), "Settings", color::black);
	}
	font_ = sdl::loadFont("Resource\\Fonts\\Kanit-Regular.ttf", 28);
	for (int i = 0; i < kRows; ++i) {
		labels_[i] = sdl::renderText(r, font_.get(), kLabels[i], color::black);
		arrows_.emplace_back(r, font_.get(), "<", SDL_Point{ kLeftX, kRowY0 + i * kRowDy }, color::black, color::red);
		arrows_.emplace_back(r, font_.get(), ">", SDL_Point{ kRightX, kRowY0 + i * kRowDy }, color::black, color::red);
		refresh(i);
	}
	{
		const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
		back_ = std::make_unique<Button>(r, font.get(), "Return", SDL_Point{ 0, 445 }, color::black, color::aqua);
		back_->centerAt(482);
	}
	if (!bg_ || !font_ || !back_->ok()) {
		g.fatal("Missing settings resources (see console)");
		go(SceneId::Quit);
	}
}

SettingsScene::~SettingsScene() { game_.save(); }

void SettingsScene::refresh(int row) {
	const Settings& s = game_.settings();
	std::string text;
	if (row == 0) text = std::to_string(s.music * 100 / 128) + "%";
	else if (row == 1) text = std::to_string(s.sfx * 100 / 128) + "%";
	else {
		text = s.fullscreen ? "On" : "Off";
		fullscreenShown_ = s.fullscreen;
	}
	values_[row] = sdl::renderText(game_.renderer(), font_.get(), text, color::black);
}

void SettingsScene::change(int row, int delta) {
	Settings& s = game_.settings();
	if (row == 0) s.music = SDL_clamp(s.music + delta * kStep, 0, 128);
	else if (row == 1) s.sfx = SDL_clamp(s.sfx + delta * kStep, 0, 128);
	else game_.setFullscreen(!s.fullscreen);
	game_.applyAudio();
	Mix_PlayChannel(-1, game_.click(), 0);  // also lets you hear the new sound volume
	refresh(row);
}

void SettingsScene::handleEvent(const SDL_Event& e) {
	if (e.type == SDL_MOUSEMOTION) {
		for (int i = 0; i < kRows; ++i) {
			if (arrows_[2 * i].hit(e.motion.x, e.motion.y) || arrows_[2 * i + 1].hit(e.motion.x, e.motion.y)) selected_ = i;
		}
		if (back_->hit(e.motion.x, e.motion.y)) selected_ = kRows;
	}
	else if (e.type == SDL_MOUSEBUTTONDOWN) {
		const int x = e.button.x, y = e.button.y;
		for (int i = 0; i < kRows; ++i) {
			if (arrows_[2 * i].hit(x, y)) change(i, -1);
			if (arrows_[2 * i + 1].hit(x, y)) change(i, +1);
		}
		if (back_->hit(x, y)) {
			Mix_PlayChannel(-1, game_.click(), 0);
			go(SceneId::Menu);
		}
	}
	else if (isKey(e, SDLK_UP) || isKey(e, SDLK_w)) {
		selected_ = (selected_ + kRows) % (kRows + 1);
	}
	else if (isKey(e, SDLK_DOWN) || isKey(e, SDLK_s)) {
		selected_ = (selected_ + 1) % (kRows + 1);
	}
	else if ((isKey(e, SDLK_LEFT) || isKey(e, SDLK_a)) && selected_ < kRows) {
		change(selected_, -1);
	}
	else if ((isKey(e, SDLK_RIGHT) || isKey(e, SDLK_d)) && selected_ < kRows) {
		change(selected_, +1);
	}
	else if (isConfirm(e)) {
		if (selected_ == kRows) go(SceneId::Menu);
		else if (selected_ == 2) change(2, 1);
	}
	else if (isKey(e, SDLK_ESCAPE)) {
		go(SceneId::Menu);
	}
}

void SettingsScene::render(SDL_Renderer* r) {
	if (game_.settings().fullscreen != fullscreenShown_) refresh(2);
	SDL_RenderCopy(r, bg_.get(), nullptr, nullptr);
	SDL_Rect t = sdl::sized(title_.get(), 0, 182);
	t.x = 482 - t.w / 2;
	SDL_RenderCopy(r, title_.get(), nullptr, &t);
	for (int i = 0; i < kRows; ++i) {
		const int y = kRowY0 + i * kRowDy;
		const SDL_Rect l = sdl::sized(labels_[i].get(), kLabelX, y);
		SDL_Rect v = sdl::sized(values_[i].get(), 0, y);
		v.x = kValueCx - v.w / 2;
		if (i == selected_) {
			SDL_SetRenderDrawColor(r, 255, 255, 255, 70);
			const SDL_Rect band{ kLabelX - 15, y - 2, kRightX + 40 - kLabelX, l.h + 4 };
			SDL_RenderFillRect(r, &band);
			SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
		}
		SDL_RenderCopy(r, labels_[i].get(), nullptr, &l);
		SDL_RenderCopy(r, values_[i].get(), nullptr, &v);
		arrows_[2 * i].setHot(i == selected_);
		arrows_[2 * i + 1].setHot(i == selected_);
		arrows_[2 * i].draw(r);
		arrows_[2 * i + 1].draw(r);
	}
	back_->setHot(selected_ == kRows);
	back_->draw(r);
}
}
