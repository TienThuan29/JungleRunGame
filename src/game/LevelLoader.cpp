#include "game/LevelLoader.h"
#include <algorithm>
#include <charconv>
#include <fstream>
#include <iterator>
#include <sstream>

namespace jr {
namespace {
constexpr int kKeyTile = 21, kCheckpointTile = 23;

bool readLines(const std::string& path, std::vector<std::string>& lines) {
	std::ifstream in(path, std::ios::binary);
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
	if (!std::ifstream(path)) {
		error = where + "cannot open " + path + " (run the game from the repo folder)";
		return -1;
	}
	Grid g{};
	if (!loadGrid(path, g, error)) return -1;
	c.gridNames.push_back(name);
	c.grids.push_back(g);
	return static_cast<int>(c.grids.size()) - 1;
}

bool hasTile(const Grid& g, int code) {
	for (const auto& row : g) {
		for (int v : row) {
			if (v == code) return true;
		}
	}
	return false;
}

// Largest x of a checkpoint in grid g placed at segment x (or -1).
float lastCheckpoint(const Grid& g, float x) {
	float best = -1;
	for (const auto& row : g) {
		for (int c = 0; c < cfg::kGridCols; ++c) {
			if (row[c] == kCheckpointTile) best = SDL_max(best, x + static_cast<float>(c * cfg::kTile));
		}
	}
	return best;
}

// True if ground platforms cover [from, to) without gaps.
bool groundCovers(std::vector<float> xs, float from, float to) {
	std::sort(xs.begin(), xs.end());
	float reach = from;
	for (float x : xs) {
		if (x <= reach && x + cfg::kGroundW > reach) reach = x + cfg::kGroundW;
	}
	return reach >= to;
}
}

bool loadCourse(const std::string& dir, const std::string& file, Course& out, std::string& error) {
	out = Course{};
	const std::string path = dir + "\\" + file;
	std::vector<std::string> lines;
	if (!readLines(path, lines)) {
		error = "cannot open " + path + " (run the game from the repo folder)";
		return false;
	}
	float cursor = 0;
	bool hasFinish = false;
	size_t finishLine = 0, arenaLine = 0;
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
			if (out.arena > 0) {
				error = where + "no map after 'arena' (the arena is plain ground)";
				return false;
			}
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
		else if (t[0] == "arena" && t.size() == 1) {
			if (out.arena > 0 || cursor <= 0) {
				error = where + "arena: only one, and not at the very start";
				return false;
			}
			out.arena = cursor;
			arenaLine = i;
		}
		else {
			error = where + "unknown line '" + lines[i] + "' (expected ground / map / onkey / finish / arena)";
			return false;
		}
	}
	out.length = cursor;
	if (hasFinish == (out.arena > 0)) {
		error = path + ": needs exactly one of 'finish <pixels>' or 'arena'";
		return false;
	}
	if (hasFinish && out.finish > out.length) {
		error = at(path, finishLine) + "finish " + std::to_string(static_cast<int>(out.finish)) +
			" is beyond course length " + std::to_string(static_cast<int>(out.length));
		return false;
	}
	if (out.arena > 0) {
		if (!groundCovers(out.groundX, out.arena, out.arena + cfg::kWindowW)) {
			error = at(path, arenaLine) + "arena needs at least 4 ground platforms right after it";
			return false;
		}
		for (size_t s = 0; s < out.segments.size(); ++s) {
			const float x = out.segments[s].x;
			float cp = lastCheckpoint(out.grids[out.segments[s].grid], x);
			if (static_cast<int>(s) == out.onKeySegment) cp = SDL_max(cp, lastCheckpoint(out.grids[out.onKeyGrid], x));
			if (cp >= out.arena) {
				error = at(path, arenaLine) + "a checkpoint (23) in " + out.gridNames[out.segments[s].grid] + " is past the arena";
				return false;
			}
		}
	}
	bool key = false;
	for (const auto& s : out.segments) key = key || hasTile(out.grids[s.grid], kKeyTile);
	if (key && out.onKeySegment < 0) SDL_Log("warning: %s: a map has a key (21) but there is no onkey line", path.c_str());
	if (!key && out.onKeySegment >= 0) SDL_Log("warning: %s: onkey is set but no map contains a key (21)", path.c_str());
	return true;
}

bool loadWorlds(const std::string& dir, std::vector<WorldInfo>& out, std::string& error) {
	out.clear();
	const std::string path = dir + "\\worlds.txt";
	std::vector<std::string> lines;
	if (!readLines(path, lines)) {
		error = "cannot open " + path + " (run the game from the repo folder)";
		return false;
	}
	for (size_t i = 0; i < lines.size(); ++i) {
		const auto t = tokens(lines[i]);
		if (t.empty()) continue;
		WorldInfo w;
		if (t.size() < 3 || !toInt(t[1], w.music) || w.music < 1 || w.music > 3) {
			error = at(path, i) + "expected '<course file> <music 1-3> <title>'";
			return false;
		}
		if (out.size() == cfg::kMaxWorlds) {
			error = at(path, i) + "at most " + std::to_string(cfg::kMaxWorlds) + " worlds";
			return false;
		}
		w.course = t[0];
		w.title = t[2];
		for (size_t k = 3; k < t.size(); ++k) w.title += " " + t[k];
		out.push_back(w);
	}
	if (out.empty()) {
		error = path + ": no worlds listed";
		return false;
	}
	return true;
}
}
