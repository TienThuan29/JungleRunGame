#pragma once
#include "game/GameAssets.h"

namespace jr {
// = drawBackground(): the scroll state is updated while drawing, so draw() is not const.
class Parallax {
public:
	void reset() {
		for (int i = 0; i < 5; ++i) {
			defaultX_[i] = 0;
			previousX_[i] = -cfg::kWindowW;
			nextX_[i] = cfg::kWindowW;
		}
	}
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) {
		constexpr int W = cfg::kWindowW, H = cfg::kWindowH;
		SDL_RenderCopy(r, a.background(0), nullptr, nullptr);
		for (int i = 1; i < 5; ++i) {
			SDL_Texture* t = a.background(i);
			const float dist = scrollX / cfg::kParallaxSpeed[i];
			const SDL_Rect prev{ static_cast<int>(dist + previousX_[i]), 0, W, H };
			const SDL_Rect def{ static_cast<int>(dist + defaultX_[i]), 0, W, H };
			const SDL_Rect next{ static_cast<int>(dist + nextX_[i]), 0, W, H };
			SDL_RenderCopy(r, t, nullptr, &def);
			SDL_RenderCopy(r, t, nullptr, &prev);
			SDL_RenderCopy(r, t, nullptr, &def);
			SDL_RenderCopy(r, t, nullptr, &def);
			SDL_RenderCopy(r, t, nullptr, &next);
			if (-dist > defaultX_[i] + W) {
				defaultX_[i] += W;
				previousX_[i] = defaultX_[i] - W;
				nextX_[i] = defaultX_[i] + W;
			}
			else if (-dist < defaultX_[i] - W) {
				defaultX_[i] -= W;
				previousX_[i] = defaultX_[i] - W;
				nextX_[i] = defaultX_[i] + W;
			}
		}
	}
	static void drawStatic(SDL_Renderer* r, const GameAssets& a) {  // ending screen (render_over)
		for (int i = 0; i < 5; ++i) SDL_RenderCopy(r, a.background(i), nullptr, nullptr);
	}
private:
	float defaultX_[5]{}, previousX_[5]{}, nextX_[5]{};
};
}
