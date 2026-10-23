// C++ side: jr::World with unloaded GameAssets (every texture/sound is null, harmless).
#include "game/GameAssets.h"
#include "game/LevelLoader.h"
#include "game/World.h"
#include "parity_api.h"
#include <cstdio>
#include <memory>

namespace {
std::unique_ptr<jr::GameAssets> assets;
std::unique_ptr<jr::Course> course;
std::unique_ptr<jr::World> world;
SDL_Renderer* renderer = nullptr;
}

extern "C" int cpp_init(SDL_Renderer* r, int difficulty) {
	renderer = r;
	if (!course) {
		course = std::make_unique<jr::Course>();
		std::string error;
		if (!jr::loadCourse("Resource\\Levels", *course, error)) {
			std::printf("loadCourse failed: %s\n", error.c_str());
			return 0;
		}
		assets = std::make_unique<jr::GameAssets>();
	}
	world = std::make_unique<jr::World>(*assets, *course, difficulty);
	world->reset();
	return 1;
}

extern "C" void cpp_place(float x, float y) { world->player().teleport(x, y); }

extern "C" void cpp_frame(ParityInput in, float dt) {  // same order as GameplayScene::run()
	world->input({ in.left != 0, in.right != 0, in.down != 0 });
	if (in.jump) world->jump();
	world->step(dt);
	world->render(renderer);
}

extern "C" void cpp_snap(ParitySnap* s) {
	const jr::PlayerState& p = world->player().state();
	s->x = p.x;
	s->y = p.y;
	s->dx = p.dx;
	s->dy = p.dy;
	s->immortalStartTime = p.immortalStartTime;
	s->bossX = world->bossX();
	s->scrollX = world->scrollX();
	s->lives = p.lives;
	s->isTakenDamage = p.isTakenDamage;
	s->isImmortal = p.isImmortal;
	s->onLedge = p.onLedge;
	s->flip = p.flip;
	s->status = p.status;
	s->potions = world->potions();
	s->keyObtained = world->keyObtained() ? 1 : 0;
	s->statusState = static_cast<int>(world->status());
	s->isOver = world->isOver() ? 1 : 0;
	s->cells = 0;
	world->level().forEachEntity([&](const jr::Entity& e) {
		if (s->cells == PARITY_MAX_CELLS) return;
		s->lethal[s->cells] = e.isLethal() ? 1 : 0;
		s->solid[s->cells] = e.isSolid() ? 1 : 0;
		s->taken[s->cells] = e.isTaken() ? 1 : 0;
		s->cellX[s->cells] = e.bounds().x;
		s->cells++;
	});
}

extern "C" void cpp_shutdown(void) {
	world.reset();
	assets.reset();
	course.reset();
}
