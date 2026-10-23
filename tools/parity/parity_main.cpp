// Frame-by-frame comparison of the C gameplay (reference) and the C++ port.
// Run from the repo root:  tools/parity/x64/Debug/ParityCheck.exe   -> "PARITY OK", exit 0.
#include "parity_api.h"
#include <SDL_mixer.h>
#include <cstdio>

namespace {
Uint64 ticks = 100000;
constexpr int kFrames = 3000;
constexpr float kDt = 0.016f;

ParityInput pattern(int kind, int f) {
	ParityInput in{};
	switch (kind) {
	case 1: in.right = 1; break;                                                                         // run right
	case 2: in.right = 1; in.jump = f % 40 == 0; break;                                                  // run + jump
	case 3: in.right = f % 90 < 60; in.left = f % 90 >= 60; in.jump = f % 25 == 0; break;                // zig-zag + jump
	case 4: in.right = 1; in.down = f % 3 == 0; break;                                                   // run + down
	default: break;                                                                                      // idle
	}
	return in;
}

bool compare(const ParitySnap& c, const ParitySnap& p, const char* scenario, int frame) {
	auto fail = [&](const char* field, double cv, double pv) {
		std::printf("MISMATCH %s frame %d %s: C=%.9g C++=%.9g\n", scenario, frame, field, cv, pv);
		return false;
	};
#define F(name) \
	if (c.name != p.name) return fail(#name, c.name, p.name)
	F(x); F(y); F(dx); F(dy); F(immortalStartTime); F(bossX); F(scrollX);
	F(lives); F(isTakenDamage); F(isImmortal); F(onLedge); F(flip); F(status);
	F(potions); F(keyObtained); F(statusState); F(isOver); F(cells);
#undef F
	for (int i = 0; i < c.cells; ++i) {
		if (c.lethal[i] != p.lethal[i] || c.solid[i] != p.solid[i] || c.taken[i] != p.taken[i] || c.cellX[i] != p.cellX[i]) {
			std::printf("MISMATCH %s frame %d cell %d (x %.1f): lethal %d/%d solid %d/%d taken %d/%d x %.9g/%.9g\n",
				scenario, frame, i, c.cellX[i], c.lethal[i], p.lethal[i], c.solid[i], p.solid[i], c.taken[i], p.taken[i],
				c.cellX[i], p.cellX[i]);
			return false;
		}
	}
	return true;
}

bool runScenario(SDL_Renderer* r, const char* name, int difficulty, int kind, bool place, float x, float y, int idleFirst) {
	static ParitySnap cs, ps;  // ~40 KB each: off the stack
	ticks += 1000;
	if (!c_init(r, difficulty) || !cpp_init(r, difficulty)) return false;
	if (place) {
		c_place(x, y);
		cpp_place(x, y);
	}
	for (int f = 0; f < kFrames; ++f) {
		const ParityInput in = f < idleFirst ? ParityInput{} : pattern(kind, f);
		ticks += 16;
		c_frame(in, kDt);
		cpp_frame(in, kDt);
		c_snap(&cs);
		cpp_snap(&ps);
		if (!compare(cs, ps, name, f)) return false;
	}
	return true;
}
}

extern "C" Uint64 parity_ticks(void) { return ticks; }

int main(int, char*[]) {
	SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
	SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		std::printf("SDL_Init: %s\n", SDL_GetError());
		return 2;
	}
	Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048);
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_CRITICAL);  // null fonts/textures would log every frame
	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 1200, 800, 32, SDL_PIXELFORMAT_ARGB8888);
	SDL_Renderer* r = SDL_CreateSoftwareRenderer(surface);

	int scenarios = 0;
	bool ok = true;
	char name[64];
	for (int kind = 0; kind < 5 && ok; ++kind) {
		SDL_snprintf(name, sizeof name, "start/pattern%d", kind);
		ok = runScenario(r, name, 5, kind, false, 0, 0, 0);
		++scenarios;
	}
	c_init(r, 5);
	const int segments = c_segment_count();
	for (int seg = 0; seg < segments && ok; ++seg) {
		for (int kind = 0; kind < 5 && ok; ++kind) {
			c_init(r, 5);
			const float x = c_segment_x(seg) + 10;
			SDL_snprintf(name, sizeof name, "segment%d/pattern%d", seg, kind);
			ok = runScenario(r, name, 5, kind, true, x, 220, 0);
			++scenarios;
		}
	}
	float kx = 0, ky = 0;
	c_init(r, 5);
	if (ok && c_find_key(&kx, &ky)) {
		ok = runScenario(r, "key", 5, 1, true, kx, ky, 300);
		++scenarios;
	}
	if (ok) {
		ok = runScenario(r, "hard/pattern2", 1, 2, false, 0, 0, 0);
		++scenarios;
	}
	if (ok) std::printf("PARITY OK: %d scenarios x %d frames\n", scenarios, kFrames);

	cpp_shutdown();
	SDL_DestroyRenderer(r);
	SDL_FreeSurface(surface);
	Mix_CloseAudio();
	SDL_Quit();
	return ok ? 0 : 1;
}
