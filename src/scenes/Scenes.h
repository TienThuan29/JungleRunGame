#pragma once
#include "game/GameAssets.h"
#include "game/LevelLoader.h"
#include "game/Tuning.h"
#include "game/World.h"
#include "scenes/Scene.h"
#include "ui/Button.h"
#include <memory>
#include <vector>

namespace jr {
namespace color {
constexpr SDL_Color black{ 0, 0, 0, 255 }, red{ 255, 0, 0, 255 }, aqua{ 0, 255, 255, 255 }, white{ 255, 255, 255, 255 },
	mint{ 110, 240, 190, 255 }, grey{ 90, 90, 90, 255 };
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
	ButtonList list_;
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
	std::vector<sdl::TexturePtr> controls_;
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
	void step(int delta);
	sdl::TexturePtr bg_, trick_, title_, mode_, note_;
	sdl::FontPtr modeFont_;
	SDL_Rect titleRect_{};
	std::unique_ptr<Button> back_, left_, right_;
};

class SettingsScene final : public Scene {
public:
	explicit SettingsScene(Game& g);
	~SettingsScene() override;
protected:
	void handleEvent(const SDL_Event& e) override;
	void render(SDL_Renderer* r) override;
private:
	void change(int row, int delta);
	void refresh(int row);
	static constexpr int kRows = 3;  // music, sounds, fullscreen; row 3 = Return
	sdl::TexturePtr bg_, title_, labels_[kRows], values_[kRows];
	sdl::FontPtr font_;
	std::vector<Button> arrows_;  // left/right per row
	std::unique_ptr<Button> back_;
	int selected_ = 0;
	bool fullscreenShown_ = false;  // F11 is handled globally: re-render the value when it changes
};

class WorldSelectScene final : public Scene {
public:
	explicit WorldSelectScene(Game& g);
protected:
	void handleEvent(const SDL_Event& e) override;
	void render(SDL_Renderer* r) override;
private:
	void choose(int i);
	sdl::TexturePtr bg_, title_, note_;
	std::vector<WorldInfo> worlds_;
	ButtonList list_;  // worlds, then Return
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
	SceneId run() override;  // fixed 60 Hz ticks, pause, results
private:
	bool loadWorld(int index);
	void reloadTuning();
	void restart();
	void record();
	bool testKeys() const;
	GameAssets assets_;  // before world_: entities borrow its pointers
	Tuning tuning_;      // before world_: World and Player keep a reference
	std::vector<WorldInfo> worlds_;
	Course course_;
	std::unique_ptr<World> world_;
	sdl::TexturePtr pauseTitle_, pauseHint_;
	int worldIndex_ = 0;
	bool paused_ = false, recorded_ = false;
};
}
