#include "scenes/Scenes.h"
#include "game/Hud.h"

namespace jr {
namespace {
const char* const kModes[3] = { "Easy", "Medium", "Hard" };
}

WorldSelectScene::WorldSelectScene(Game& g) : Scene(g, SceneId::WorldSelect) {
	g.resize(cfg::kMenuW, cfg::kMenuH);
	SDL_Renderer* r = g.renderer();
	bg_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\ChooseBG.bmp");
	std::string error;
	if (!loadWorlds("Resource\\Levels", worlds_, error)) {
		g.fatal(error + "\nReturning to menu.");
		go(SceneId::Menu);
		return;
	}
	const Settings& s = g.settings();
	{
		const sdl::FontPtr big = sdl::loadFont("Resource\\Fonts\\crazy-pixel.ttf", 50);
		title_ = sdl::renderText(r, big.get(), std::string("Select World - ") + kModes[s.modeIndex], color::black);
	}
	{
		const sdl::FontPtr small = sdl::loadFont("Resource\\Fonts\\Kanit-Light.ttf", 20);
		if (s.modeIndex == 2) note_ = sdl::renderText(r, small.get(), "Hard: no checkpoints", color::black);
	}
	const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\Kanit-Regular.ttf", 26);
	const int n = static_cast<int>(worlds_.size());
	list_.items.reserve(static_cast<size_t>(n) + 1);
	for (int i = 0; i < n; ++i) {
		std::string text = std::to_string(i + 1) + ".  ";
		const bool open = i < s.unlocked;
		if (open) {
			text += worlds_[i].title;
			const Uint32 best = s.best[i][s.modeIndex];
			if (best) text += "      best " + Hud::clock(best);
		}
		else {
			text += "[locked - finish world " + std::to_string(i) + "]";
		}
		list_.items.emplace_back(r, font.get(), text.c_str(), SDL_Point{ 0, 255 + i * 48 }, open ? color::black : color::grey,
			open ? color::red : color::grey);
		list_.items.back().centerAt(482);
	}
	{
		const sdl::FontPtr small = sdl::loadFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
		list_.items.emplace_back(r, small.get(), "Return", SDL_Point{ 0, 445 }, color::black, color::aqua);
		list_.items.back().centerAt(482);
	}
	list_.selected = SDL_min(s.world, n - 1);
	if (!bg_ || !list_.items[0].ok()) {
		g.fatal("Missing world select resources (see console)");
		go(SceneId::Quit);
	}
}

void WorldSelectScene::choose(int i) {
	const int n = static_cast<int>(worlds_.size());
	if (i == n) {
		go(SceneId::Menu);
		return;
	}
	Settings& s = game_.settings();
	if (i >= s.unlocked) return;
	s.world = i;
	Mix_HaltMusic();
	go(i == 0 && !s.introSeen ? SceneId::Intro : SceneId::Gameplay);
}

void WorldSelectScene::handleEvent(const SDL_Event& e) {
	if (isKey(e, SDLK_ESCAPE)) {
		go(SceneId::Menu);
		return;
	}
	const int i = list_.handle(e, game_.click());
	if (i >= 0) choose(i);
}

void WorldSelectScene::render(SDL_Renderer* r) {
	SDL_RenderCopy(r, bg_.get(), nullptr, nullptr);
	SDL_Rect t = sdl::sized(title_.get(), 0, 185);
	t.x = 482 - t.w / 2;
	SDL_RenderCopy(r, title_.get(), nullptr, &t);
	if (note_) {
		SDL_Rect d = sdl::sized(note_.get(), 0, 405);
		d.x = 482 - d.w / 2;
		SDL_RenderCopy(r, note_.get(), nullptr, &d);
	}
	list_.draw(r);
}
}
