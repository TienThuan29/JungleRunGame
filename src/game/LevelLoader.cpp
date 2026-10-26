#include "game/LevelLoader.h"
#include "core/SdlHandles.h"
#include <charconv>
#include <fstream>
#include <iterator>
#include <sstream>

namespace jr {
namespace {
constexpr int kKeyTile = 21;

bool readLines(const std::string& path, std::vector<std::string>& lines) {
	std::ifstream in(sdl::normalizePath(path), std::ios::binary);
	if (!in) return false;
	std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	if (text.compare(0, 3, "\xEF\xBB\xBF") == 0) text.erase(0, 3);  // UTF-8 BOM
	std::istringstream ss(text);
	std::string line;
	while (std::getline(ss, line)) {
		if (!line.empty() && line.back() == '\r') line.pop_back();     // CRLF
		lines.push_back(line);
	}
	return true;
}

std::vector<std::string> tokens(const std::string& line) {
	std::istringstream ss(line.substr(0, line.find('#')));
	std::vector<std::string> out;
	std::string t;
	while (ss >> t) out.push_back(t);
	return out;
}

bool toInt(const std::string& s, int& v) {
	const char* end = s.data() + s.size();
	const auto [p, ec] = std::from_chars(s.data(), end, v);
	return ec == std::errc() && p == end;
}

std::string at(const std::string& path, size_t lineIndex) {
	return path + " line " + std::to_string(lineIndex + 1) + ": ";
}

bool loadGrid(const std::string& path, Grid& grid, std::string& error) {
	std::vector<std::string> lines;
	if (!readLines(path, lines)) {
		error = "cannot open " + path;
		return false;
	}
	int row = 0;
	for (size_t i = 0; i < lines.size(); ++i) {
		const auto t = tokens(lines[i]);
		if (t.empty()) continue;
		if (row == cfg::kGridRows) {
			error = at(path, i) + "more than 16 rows";
			return false;
		}
		if (t.size() != cfg::kGridCols) {
			error = at(path, i) + "expected 24 numbers, found " + std::to_string(t.size());
			return false;
		}
		for (int c = 0; c < cfg::kGridCols; ++c) {
			int v = 0;
			if (!toInt(t[c], v) || v < 0 || v > cfg::kMaxTileCode) {
				error = at(path, i) + "unknown tile code " + t[c] + " (valid 0-" + std::to_string(cfg::kMaxTileCode) + ")";
				return false;
			}
			grid[row][c] = v;
		}
		++row;
	}
	if (row != cfg::kGridRows) {
		error = path + ": expected 16 rows, found " + std::to_string(row);
		return false;
	}
	return true;
}

int gridIndex(Course& c, const std::string& dir, const std::string& name, const std::string& where, std::string& error) {
	for (size_t i = 0; i < c.gridNames.size(); ++i) {
		if (c.gridNames[i] == name) return static_cast<int>(i);
	}
	const std::string path = dir + "\\" + name + ".txt";
	if (!std::ifstream(sdl::normalizePath(path))) {
		error = where + "cannot open " + path + " (run the game from the repo folder)";
		return -1;
	}
	Grid g{};
	if (!loadGrid(path, g, error)) return -1;
	c.gridNames.push_back(name);
	c.grids.push_back(g);
	return static_cast<int>(c.grids.size()) - 1;
}

bool hasKey(const Grid& g) {
	for (const auto& row : g) {
		for (int v : row) {
			if (v == kKeyTile) return true;
		}
	}
	return false;
}
}

bool loadCourse(const std::string& dir, Course& out, std::string& error) {
	out = Course{};
	const std::string path = dir + "\\course.txt";
	std::vector<std::string> lines;
	if (!readLines(path, lines)) {
		error = "cannot open " + path + " (run the game from the repo folder)";
		return false;
	}
	float cursor = 0;
	bool hasFinish = false;
	size_t finishLine = 0;
	for (size_t i = 0; i < lines.size(); ++i) {
		const auto t = tokens(lines[i]);
		if (t.empty()) continue;
		const std::string where = at(path, i);
		int n = 0;
		if (t[0] == "ground" && t.size() == 2) {
			if (!toInt(t[1], n) || n < 1) {
				error = where + "ground needs a positive count";
				return false;
			}
			for (int k = 0; k < n; ++k) {
				out.groundX.push_back(cursor);
				cursor += cfg::kGroundW;
			}
		}
		else if (t[0] == "map" && t.size() >= 2) {
			for (size_t k = 1; k < t.size(); ++k) {
				const int g = gridIndex(out, dir, t[k], where, error);
				if (g < 0) return false;
				out.segments.push_back({ cursor, g });
				cursor += cfg::kSegmentW;
			}
		}
		else if (t[0] == "onkey" && t.size() == 3) {
			if (out.onKeySegment >= 0) {
				error = where + "only one onkey line is allowed";
				return false;
			}
			int found = -1;
			for (size_t s = 0; s < out.segments.size(); ++s) {
				if (out.gridNames[out.segments[s].grid] != t[1]) continue;
				if (found >= 0) {
					error = where + "onkey: " + t[1] + " is used by more than one map line";
					return false;
				}
				found = static_cast<int>(s);
			}
			if (found < 0) {
				error = where + "onkey: " + t[1] + " is not a map declared above";
				return false;
			}
			const int g = gridIndex(out, dir, t[2], where, error);
			if (g < 0) return false;
			out.onKeySegment = found;
			out.onKeyGrid = g;
		}
		else if (t[0] == "finish" && t.size() == 2) {
			if (!toInt(t[1], n) || n <= 0) {
				error = where + "finish needs a positive pixel position";
				return false;
			}
			out.finish = static_cast<float>(n);
			hasFinish = true;
			finishLine = i;
		}
		else {
			error = where + "unknown line '" + lines[i] + "' (expected ground / map / onkey / finish)";
			return false;
		}
	}
	out.length = cursor;
	if (!hasFinish) {
		error = path + ": missing 'finish <pixels>' line";
		return false;
	}
	if (out.finish > out.length) {
		error = at(path, finishLine) + "finish " + std::to_string(static_cast<int>(out.finish)) +
			" is beyond course length " + std::to_string(static_cast<int>(out.length));
		return false;
	}
	bool key = false;
	for (const auto& s : out.segments) key = key || hasKey(out.grids[s.grid]);
	if (key && out.onKeySegment < 0) SDL_Log("warning: %s: a map has a key (21) but there is no onkey line", path.c_str());
	if (!key && out.onKeySegment >= 0) SDL_Log("warning: %s: onkey is set but no map contains a key (21)", path.c_str());
	return true;
}
}
