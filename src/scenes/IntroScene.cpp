#include "scenes/Scenes.h"
#include <cstring>

namespace jr {
namespace {
const char* const kStory[7] = {
	"\"...In 2016, Chris, who wanted to cure his beloved sister Claire's late-stage cancer, set out for the",
	"forest after obtaining the location from a lucky explorer who survived the forest. He hoped to find the",
	"treasure in the legend and use its power to save his sister. He crossed through dangerous creatures and",
	"reached the relic. He advanced deep into the relic, trying to hide and find a way to defeat the Golem.",
	"finally. The Golem chased after him and used its power to trap him in an endless loop of being pursued",
	"until death. Chris, with no weapons except for the sword he picked up from the relic, had to break the",
	"loop and defeat the Golem to reach the treasure.\""
};
}

IntroScene::IntroScene(Game& g) : Scene(g, SceneId::Intro) {
	g.resize(cfg::kWindowW, cfg::kWindowH);
	SDL_Renderer* r = g.renderer();
	typing_ = sdl::loadMusic("Resource\\Sound\\Sound Effects\\typing.mp3");
	Mix_PlayMusic(typing_.get(), -1);
	g.settings().introSeen = true;  // later runs of world 1 skip the intro
	g.save();
	{
		const sdl::FontPtr big = sdl::loadFont("Resource\\Fonts\\crazy-pixel.ttf", 80);
		skip_ = sdl::renderText(r, big.get(), ">> Press [SPACE] to skip <<", color::white);
	}
	font_ = sdl::loadFont("Resource\\Fonts\\Kanit-Light.ttf", 20);
	black_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\bg.bmp");
	whitebar_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\whitebar.bmp");
	blackbar_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\blackbar.bmp");
	if (!font_ || !black_ || !whitebar_ || !blackbar_) {
		g.fatal("Missing intro resources (see console)");
		go(SceneId::Quit);
	}
}

IntroScene::~IntroScene() { Mix_HaltMusic(); }  // intro.c:86, before typing_ is freed

// Typewriter intro. Draws incrementally on purpose: earlier lines stay on screen,
// each step only repaints from the current line down (same as the C version).
SceneId IntroScene::run() {
	SDL_Renderer* r = game_.renderer();
	const SDL_Rect skipRect = sdl::sized(skip_.get(), 290, 500);
	SDL_Rect lineRect{ 120, 76, 0, 0 }, underline{ 100, 100, 15, 5 }, blackRect{ 0, 100, 1200, 800 };
	int charNumber = 0, lineNumber = 0;
	SDL_RenderClear(r);
	while (running()) {
		SDL_Event e;
		while (running() && SDL_PollEvent(&e)) {
			if (game_.handleGlobal(e)) continue;
			if (e.type == SDL_QUIT) go(SceneId::Quit);
			else if (isKey(e, SDLK_ESCAPE)) go(SceneId::Menu);
			else if (isConfirm(e)) go(SceneId::Gameplay);
		}
		if (!running()) break;
		if (charNumber >= static_cast<int>(std::strlen(kStory[lineNumber]))) {
			if (++lineNumber >= 7) {
				go(SceneId::Gameplay);
				break;
			}
			charNumber = 0;
			underline.y += 40;
			underline.x = 100;
			blackRect.y += 40;
			lineRect.y += 40;
		}
		charNumber += 5;
		char prefix[150];
		SDL_strlcpy(prefix, kStory[lineNumber], SDL_min(sizeof prefix, static_cast<size_t>(charNumber) + 1));
		line_ = sdl::renderText(r, font_.get(), prefix, color::white);
		lineRect = sdl::sized(line_.get(), lineRect.x, lineRect.y);
		underline.x += 44;

		SDL_Delay(100);
		SDL_RenderCopy(r, black_.get(), nullptr, &blackRect);
		SDL_RenderCopy(r, line_.get(), nullptr, &lineRect);
		SDL_RenderCopy(r, skip_.get(), nullptr, &skipRect);
		SDL_RenderPresent(r);
		SDL_Delay(60);
		SDL_RenderCopy(r, underline.x >= 1000 ? blackbar_.get() : whitebar_.get(), nullptr, &underline);
		SDL_RenderPresent(r);
		game_.tick();
	}
	return next_;
}
}
