#include "ui/Button.h"

namespace jr {
Button::Button(SDL_Renderer* r, TTF_Font* font, const char* text, SDL_Point at, SDL_Color normal, SDL_Color hot)
	: normal_(sdl::renderText(r, font, text, normal)), hotTex_(sdl::renderText(r, font, text, hot)),
	  rect_(sdl::sized(normal_.get(), at.x, at.y)) {}

int ButtonList::handle(const SDL_Event& e, Mix_Chunk* click) {
	const int n = static_cast<int>(items.size());
	int chosen = -1;
	if (e.type == SDL_MOUSEMOTION) {
		for (int i = 0; i < n; ++i) {
			if (items[i].hit(e.motion.x, e.motion.y)) selected = i;
		}
	}
	else if (e.type == SDL_MOUSEBUTTONDOWN) {
		for (int i = 0; i < n; ++i) {
			if (items[i].hit(e.button.x, e.button.y)) chosen = selected = i;
		}
	}
	else if (e.type == SDL_KEYDOWN) {
		const SDL_Keycode k = e.key.keysym.sym;
		if (k == SDLK_UP || k == SDLK_w) selected = (selected + n - 1) % n;
		else if (k == SDLK_DOWN || k == SDLK_s) selected = (selected + 1) % n;
		else if (k == SDLK_RETURN || k == SDLK_KP_ENTER || k == SDLK_SPACE) chosen = selected;
	}
	if (chosen >= 0) Mix_PlayChannel(-1, click, 0);
	return chosen;
}

void ButtonList::draw(SDL_Renderer* r) {
	for (int i = 0; i < static_cast<int>(items.size()); ++i) {
		items[i].setHot(i == selected);
		items[i].draw(r);
	}
}
}
