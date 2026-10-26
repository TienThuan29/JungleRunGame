// Loads worlds.txt, every course it lists and tuning.txt with the game's own loaders and checks
// the rules the game relies on. No window, no sound. Build and run: tools\selfcheck.bat
#define SDL_MAIN_HANDLED
#include "game/LevelLoader.h"
#include "game/Tuning.h"
#include <algorithm>
#include <cstdio>
#include <fstream>

using namespace jr;

namespace {
int failures = 0;

void check(bool ok, const std::string& what) {
	std::printf("%s %s\n", ok ? "ok  " : "FAIL", what.c_str());
	if (!ok) ++failures;
}

int count(const Grid& g, int code) {
	int n = 0;
	for (const auto& row : g)
		for (int v : row) if (v == code) ++n;
	return n;
}
}

int main() {
	const std::string dir = "Resource\\Levels";
	std::vector<WorldInfo> worlds;
	std::string error;
	check(loadWorlds(dir, worlds, error), "worlds.txt loads " + error);
	check(worlds.size() == 3, "3 worlds");
	for (const WorldInfo& w : worlds) {
		Course c;
		error.clear();
		const bool ok = loadCourse(dir, w.course, c, error);
		check(ok, w.course + " loads " + error);
		if (!ok) continue;
		int potions = 0, checkpoints = 0;
		for (size_t s = 0; s < c.segments.size(); ++s) {
			const Grid& g = static_cast<int>(s) == c.onKeySegment ? c.grids[c.onKeyGrid] : c.grids[c.segments[s].grid];
			potions += count(g, 14);
			checkpoints += count(g, 23);
		}
		check(potions >= 5, w.course + ": " + std::to_string(potions) + " potions reachable (5 for the Good End)");
		check(checkpoints > 0, w.course + ": " + std::to_string(checkpoints) + " checkpoints");
		check(c.goal() > 0 && c.goal() <= c.length, w.course + ": goal inside the course");
	}
	{
		Course c;
		error.clear();
		check(loadCourse(dir, "course3.txt", c, error) && c.arena > 0 && c.finish == 0, "course3.txt ends in an arena");
	}

	Tuning t;
	std::string problems;
	loadTuning("Resource\\tuning.txt", t, problems);
	check(problems.empty(), "tuning.txt has no problems " + problems);
	const Tuning d;
	check(t.jumpVy == d.jumpVy && t.gravity == d.gravity && t.bossHp == d.bossHp && t.debug == 0, "tuning.txt ships the defaults, debug off");

	const char* bad = "x64\\selfcheck_tuning.txt";
	std::ofstream(bad) << "gravity 9\njump_vy abc\nnope 1\nboss_hp 7 # comment\n";
	loadTuning(bad, t, problems);
	check(t.gravity == d.gravity && t.jumpVy == d.jumpVy && t.bossHp == 7, "bad tuning values fall back to defaults");
	check(std::count(problems.begin(), problems.end(), '\n') == 3, "three tuning problems reported");

	std::printf(failures ? "\n%d check(s) FAILED\n" : "\nall checks passed\n", failures);
	return failures ? 1 : 0;
}
