#pragma once
#include <SDL.h>
#include <array>

namespace jr {
// Fixed pool of square sparks; when full the oldest is overwritten. Never allocates.
class Particles {
public:
	void clear() { p_ = {}; }
	void burst(float x, float y, int n, SDL_Color c, float speed) {
		for (int i = 0; i < n; ++i) {
			P& q = p_[next_];
			next_ = (next_ + 1) % p_.size();
			const float a = rnd() * 6.2832f, s = speed * (0.4f + 0.6f * rnd());
			q = { x, y, SDL_cosf(a) * s, SDL_sinf(a) * s - speed * 0.5f, 0, 0.35f + 0.35f * rnd(), c };
		}
	}
	void update(float dt) {
		for (P& q : p_) {
			if (q.age >= q.life) continue;
			q.age += dt;
			q.x += q.dx * dt;
			q.y += q.dy * dt;
			q.dy += 600 * dt;
		}
	}
	void draw(SDL_Renderer* r, float scrollX) const {
		for (const P& q : p_) {
			if (q.age >= q.life) continue;
			const float k = 1 - q.age / q.life;
			SDL_SetRenderDrawColor(r, q.c.r, q.c.g, q.c.b, static_cast<Uint8>(255 * k));
			const SDL_Rect d{ static_cast<int>(scrollX + q.x) - 3, static_cast<int>(q.y) - 3, 6, 6 };
			SDL_RenderFillRect(r, &d);
		}
		SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
	}
private:
	struct P {
		float x, y, dx, dy, age, life;
		SDL_Color c;
	};
	float rnd() {
		seed_ = seed_ * 1664525u + 1013904223u;
		return static_cast<float>(seed_ >> 8) / 16777216.0f;
	}
	std::array<P, 256> p_{};
	size_t next_ = 0;
	Uint32 seed_ = 12345;
};
}
