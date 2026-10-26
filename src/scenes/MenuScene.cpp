#include "scenes/Scenes.h"

namespace jr {
namespace {
constexpr int kItems = 7;
const char* const kLabels[kItems] = { "Play", "Story", "Game Mode", "Settings", "Help", "Donate", "Exit" };
const SceneId kTarget[kItems] = { SceneId::WorldSelect, SceneId::Story, SceneId::GameMode, SceneId::Settings,
	SceneId::Help, SceneId::Donate, SceneId::Quit };
}

MenuScene::MenuScene(Game& g) : Scene(g, SceneId::Menu) {
	g.resize(cfg::kMenuW, cfg::kMenuH);
	if (!Mix_PlayingMusic()) Mix_PlayMusic(g.menuMusic(), -1);
	SDL_Renderer* r = g.renderer();
	bg_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\ChooseBG.bmp");
	const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\crazy-pixel.ttf", 50);
	list_.items.reserve(kItems);
	for (int i = 0; i < kItems; ++i) {
		list_.items.emplace_back(r, font.get(), kLabels[i], SDL_Point{ 0, 168 + i * 47 }, color::black, color::red);
		list_.items.back().centerAt(482);
	}
	if (!bg_ || !list_.items[0].ok()) {
		g.fatal("Missing menu resources (see console)");
		go(SceneId::Quit);
	}
}

MenuScene::~MenuScene() = default;

void MenuScene::handleEvent(const SDL_Event& e) {
	const int i = list_.handle(e, game_.click());  // Esc does nothing here: gamepad B is Esc, Exit quits
	if (i >= 0) go(kTarget[i]);
}

void MenuScene::render(SDL_Renderer* r) {
	SDL_RenderCopy(r, bg_.get(), nullptr, nullptr);
	list_.draw(r);
}
}
