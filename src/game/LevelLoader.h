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
	float finish = 0;
	float length = 0;
};

// Parses <dir>\course.txt and every grid it names. On failure returns false and
// sets `error` to an English message with file path and 1-based physical line.
bool loadCourse(const std::string& dir, Course& out, std::string& error);
}
