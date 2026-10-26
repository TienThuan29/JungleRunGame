#pragma once
#include "core/Game.h"

namespace jr {
// Template Method loop for the menu scenes. Intro and Gameplay override run() to keep the C loop order.
// Resources are members: they are released when Game destroys the scene.
class Scene {
public:
	Scene(Game& game, SceneId self) : game_(game), self_(self), next_(self) {}
	virtual ~Scene() = default;
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
	virtual SceneId run();
protected:
	virtual void handleEvent(const SDL_Event&) {}
	virtual void update(float) {}
	virtual void render(SDL_Renderer*) {}
	void go(SceneId next) { next_ = next; }  // stop polling; leftover events stay queued for the next scene
	bool running() const { return next_ == self_; }
	// Black overlay fading out over `secs` after `startMs` (scene entry).
	static void drawFade(SDL_Renderer* r, Uint64 startMs, float secs);
	static bool isKey(const SDL_Event& e, SDL_Keycode k) { return e.type == SDL_KEYDOWN && e.key.keysym.sym == k; }
	static bool isConfirm(const SDL_Event& e) { return isKey(e, SDLK_RETURN) || isKey(e, SDLK_KP_ENTER) || isKey(e, SDLK_SPACE); }
	Game& game_;
	const SceneId self_;
	SceneId next_;
};
}
