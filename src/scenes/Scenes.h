#pragma once
#include "game/GameAssets.h"
#include "game/LevelLoader.h"
#include "game/World.h"
#include "scenes/Scene.h"
#include "ui/Button.h"
#include <memory>
#include <vector>

namespace jr {
namespace color {
constexpr SDL_Color black{ 0, 0, 0, 255 }, red{ 255, 0, 0, 255 }, aqua{ 0, 255, 255, 255 }, white{ 255, 255, 255, 255 };
}

class MenuScene final : public Scene {
public:
	explicit MenuScene(Game& g);
	~MenuScene() override;
protected:
	void handleEvent(const SDL_Event& e) override;
	void render(SDL_Renderer* r) override;
private:
	sdl::TexturePtr bg_;
	std::vector<Button> items_;
};

class StoryScene final : public Scene {
public:
	explicit StoryScene(Game& g);
protected:
	void handleEvent(const SDL_Event& e) override;
	void render(SDL_Renderer* r) override;
private:
	sdl::TexturePtr page_[2], red_, green_;
	std::unique_ptr<Button> back_;
	int current_ = 0;
	bool hotRight_ = false, hotLeft_ = false;
};

class HelpScene final : public Scene {
public:
	explicit HelpScene(Game& g);
protected:
	void handleEvent(const SDL_Event& e) override;
	void render(SDL_Renderer* r) override;
private:
	sdl::TexturePtr bg_, fire_, saw_, ground_, lightning_, ceiling_;
	std::array<sdl::TexturePtr, 7> health_;
	std::array<sdl::TexturePtr, 8> good_, running_;
	std::array<sdl::TexturePtr, 5> jumping_;
	std::unique_ptr<Button> back_;
};

class DonateScene final : public Scene {
public:
	explicit DonateScene(Game& g);
protected:
	void handleEvent(const SDL_Event& e) override;
	void render(SDL_Renderer* r) override;
private:
	sdl::TexturePtr bg_, qr_, logo_, thanks_;
	SDL_Rect thankRect_{};
	std::unique_ptr<Button> back_;
};

class GameModeScene final : public Scene {
public:
	explicit GameModeScene(Game& g);
protected:
	void handleEvent(const SDL_Event& e) override;
	void render(SDL_Renderer* r) override;
private:
	void refreshMode();
	sdl::TexturePtr bg_, trick_, title_, mode_;
	sdl::FontPtr modeFont_;
	SDL_Rect titleRect_{};
	std::unique_ptr<Button> back_, left_, right_;
};

class IntroScene final : public Scene {
public:
	explicit IntroScene(Game& g);
	~IntroScene() override;
	SceneId run() override;  // C loop kept: incremental drawing with SDL_Delay, no clear
private:
	sdl::MusicPtr typing_;
	sdl::FontPtr font_;
	sdl::TexturePtr skip_, black_, whitebar_, blackbar_, line_;
};

class GameplayScene final : public Scene {
public:
	explicit GameplayScene(Game& g);
	~GameplayScene() override;
	SceneId run() override;  // C loop kept: keyboard state before events
private:
	GameAssets assets_;  // before world_: entities borrow its pointers
	Course course_;
	std::unique_ptr<World> world_;
};
}
