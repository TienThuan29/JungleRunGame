#include "scenes/Scene.h"

namespace jr {
SceneId Scene::run() {
	SDL_Renderer* r = game_.renderer();
	Uint64 last = SDL_GetTicks64();
	while (running()) {
		SDL_Event e;
		while (running() && SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) go(SceneId::Quit);
			else handleEvent(e);
		}
		if (!running()) break;
		const Uint64 now = SDL_GetTicks64();
		update(static_cast<float>(now - last) / 1000.0f);
		last = now;
		SDL_RenderClear(r);
		render(r);
		SDL_RenderPresent(r);
		game_.tick();
	}
	return next_;
}
}
