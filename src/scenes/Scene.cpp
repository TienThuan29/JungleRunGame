#include "scenes/Scene.h"

namespace jr {
void Scene::drawFade(SDL_Renderer* r, Uint64 startMs, float secs) {
	const float k = 1.0f - static_cast<float>(SDL_GetTicks64() - startMs) / 1000.0f / secs;
	if (k <= 0) return;
	SDL_SetRenderDrawColor(r, 0, 0, 0, static_cast<Uint8>(255 * SDL_min(k, 1.0f)));
	SDL_RenderFillRect(r, nullptr);
	SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
}

SceneId Scene::run() {
	SDL_Renderer* r = game_.renderer();
	const Uint64 start = SDL_GetTicks64();
	Uint64 last = start;
	while (running()) {
		SDL_Event e;
		while (running() && SDL_PollEvent(&e)) {
			if (game_.handleGlobal(e)) continue;
			if (e.type == SDL_QUIT) go(SceneId::Quit);
			else handleEvent(e);
		}
		if (!running()) break;
		const Uint64 now = SDL_GetTicks64();
		update(static_cast<float>(now - last) / 1000.0f);
		last = now;
		SDL_RenderClear(r);
		render(r);
		drawFade(r, start, 0.15f);
		SDL_RenderPresent(r);
		game_.tick();
	}
	return next_;
}
}
