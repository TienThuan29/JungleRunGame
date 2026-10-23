#include "game/World.h"

namespace jr {
namespace {
float nowSec() { return static_cast<float>(SDL_GetTicks64()) / 1000.0f; }
}

void World::reset() {
	status_ = Status::Playing;
	isOver_ = false;
	overTiming_ = 0;
	player_.reset(difficulty_ == cfg::kDifficultyMedium ? cfg::kDifficultyMedium
		: difficulty_ == cfg::kDifficultyHard ? cfg::kDifficultyHard : cfg::kDifficultyEasy);
	potions_ = 0;
	keyObtained_ = false;
	swapped_ = false;
	bossStart_ = nowSec();
	boss_.reset();
	scrollX_ = 0;
	parallax_.reset();
	hud_.resetOver();
	level_.build(course_);
}

void World::step(float dt) {
	level_.collideGround(player_, assets_.landing());
	level_.collideSegments(player_, *this, dt);
	physics(dt);
}

void World::physics(float dt) {
	const PlayerState& s = player_.state();
	if (s.x >= course_.finish && !isOver_) {
		isOver_ = true;
		Mix_HaltMusic();
		overTiming_ = nowSec();
		if (potions_ < cfg::kRequiredPotions) status_ = Status::TrueEnd;
		else if (potions_ == cfg::kRequiredPotions) status_ = Status::GoodEnd;
	}
	else if ((boss_.touches(player_) || s.lives == 0 || s.y > cfg::kWindowH + 250) && !isOver_) {
		isOver_ = true;
		Mix_HaltChannel(-1);
		Mix_HaltChannel(1);
		Mix_HaltMusic();
		overTiming_ = nowSec();
		status_ = Status::GameOver;
	}
	else {
		if (s.isImmortal) player_.updateImmortal(nowSec());
		player_.integrate(assets_.jumpSound());
		scrollX_ = -player_.state().x + 350;
		if (scrollX_ > 0) scrollX_ = 0;
		if (nowSec() - bossStart_ >= 0.5f) boss_.update(dt, scrollX_, player_.state().dx);
	}
}

bool World::overReady() const { return isOver_ && nowSec() - overTiming_ >= 2.0f; }

void World::renderOver(SDL_Renderer* r, const char* title, SDL_Color fg) {
	Mix_HaltChannel(-1);
	if (!Mix_PlayingMusic()) Mix_PlayMusic(assets_.endingMusic(static_cast<int>(status_)), -1);
	Parallax::drawStatic(r, assets_);
	hud_.drawOver(r, assets_, title, fg, nowSec() - overTiming_ >= 2.0f);
}

void World::render(SDL_Renderer* r) {
	SDL_RenderClear(r);
	switch (status_) {
	case Status::GameOver: renderOver(r, "Bad End", SDL_Color{ 210, 8, 45, 255 }); break;
	case Status::TrueEnd: renderOver(r, "True End", SDL_Color{ 200, 112, 126, 255 }); break;
	case Status::GoodEnd: renderOver(r, "Good End", SDL_Color{ 96, 106, 116, 255 }); break;
	case Status::Playing:
		parallax_.draw(r, assets_, scrollX_);
		if (keyObtained_ && !swapped_) {  // = draw_key_map in do_render, before the maps are drawn
			level_.swapOnKey(course_);
			swapped_ = true;
		}
		level_.animate(SDL_GetTicks64());
		level_.drawSegments(r, assets_, scrollX_);
		level_.drawGrounds(r, assets_, scrollX_);
		player_.draw(r, assets_, scrollX_);
		boss_.draw(r, assets_, scrollX_);
		hud_.drawStatus(r, assets_, player_.state().lives, potions_);
		if (!Mix_PlayingMusic()) Mix_PlayMusic(assets_.inGameMusic(), -1);
		break;
	}
	SDL_RenderPresent(r);
}
}
