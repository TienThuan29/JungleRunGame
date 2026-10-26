#pragma once
#include "core/SdlHandles.h"
#include <vector>

namespace jr {
// Text that switches to the `hot` color under the mouse or when selected with the keyboard.
// Hit test is inclusive, like ui_hit() in the C version.
class Button {
public:
	Button(SDL_Renderer* r, TTF_Font* font, const char* text, SDL_Point at, SDL_Color normal, SDL_Color hot);
	bool ok() const { return normal_ != nullptr; }
	void hover(int x, int y) { hot_ = hit(x, y); }
	void setHot(bool hot) { hot_ = hot; }
	void centerAt(int cx) { rect_.x = cx - rect_.w / 2; }
	bool hit(int x, int y) const { return x >= rect_.x && x <= rect_.x + rect_.w && y >= rect_.y && y <= rect_.y + rect_.h; }
	void draw(SDL_Renderer* r) const { SDL_RenderCopy(r, hot_ ? hotTex_.get() : normal_.get(), nullptr, &rect_); }
	const SDL_Rect& rect() const { return rect_; }
private:
	sdl::TexturePtr normal_, hotTex_;
	SDL_Rect rect_{};
	bool hot_ = false;
};
// Vertical list of buttons driven by mouse or Up/Down + Enter/Space. Returns the activated index or -1.
class ButtonList {
public:
	std::vector<Button> items;
	int handle(const SDL_Event& e, Mix_Chunk* click);
	void draw(SDL_Renderer* r);
	int selected = 0;
};

}
