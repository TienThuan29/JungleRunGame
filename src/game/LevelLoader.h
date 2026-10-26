#pragma once
#include "game/Config.h"
#include <array>
#include <string>
#include <vector>

namespace jr {
using Grid = std::array<std::array<int, cfg::kGridCols>, cfg::kGridRows>;

struct Course {
	struct Segment {
		float x;
		int grid;  // index into grids
	};
	std::vector<float> groundX;  // left edge of each ground platform
	std::vector<Segment> segments;
	std::vector<std::string> gridNames;
	std::vector<Grid> grids;
	int onKeySegment = -1;  // segment rebuilt when the key is picked up
	int onKeyGrid = -1;     // grid it is rebuilt from
	float finish = 0;       // run ends at x >= finish (0 when the course has an arena)
	float arena = 0;        // boss fight starts at x >= arena (0 = no fight)
	float length = 0;
	float goal() const { return arena > 0 ? arena : finish; }
};

struct WorldInfo {
	std::string course;  // file name in the levels folder
	int music = 3;       // In Game 1-3
	std::string title;
};

// Parses <dir>\<file> and every grid it names. On failure returns false and
// sets `error` to an English message with file path and 1-based physical line.
bool loadCourse(const std::string& dir, const std::string& file, Course& out, std::string& error);

// Parses <dir>\worlds.txt: one line per world, `<course file> <music 1-3> <title...>`, 1 to 3 lines.
bool loadWorlds(const std::string& dir, std::vector<WorldInfo>& out, std::string& error);
}
