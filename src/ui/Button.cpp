#include "ui/Button.h"

namespace jr {
Button::Button(SDL_Renderer* r, TTF_Font* font, const char* text, SDL_Point at, SDL_Color normal, SDL_Color hot)
	: normal_(sdl::renderText(r, font, text, normal)), hotTex_(sdl::renderText(r, font, text, hot)),
	  rect_(sdl::sized(normal_.get(), at.x, at.y)) {}
}
