#include "core/SaveFile.h"
#include <charconv>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace jr {
namespace {
std::filesystem::path savePath() {  // SDL returns UTF-8; u8path keeps non-ASCII folders working on Windows
	const sdl::SdlString base(SDL_GetBasePath());
	return base ? std::filesystem::u8path(base.get()) / "save.txt" : std::filesystem::path("save.txt");
}

bool toInt(const std::string& s, long long& v) {
	const char* end = s.data() + s.size();
	const auto [p, ec] = std::from_chars(s.data(), end, v);
	return ec == std::errc() && p == end;
}

std::string bestKey(int world, int mode) { return "best_" + std::to_string(world + 1) + "_" + std::to_string(mode); }
}

void loadSettings(Settings& s) {
	s = Settings{};
	std::ifstream in(savePath());
	std::string line;
	while (in && std::getline(in, line)) {
		std::istringstream ss(line);
		std::string key, value;
		long long v = 0;
		if (!(ss >> key >> value) || !toInt(value, v)) continue;
		const auto in01 = [&](long long lo, long long hi) { return v >= lo && v <= hi; };
		if (key == "music" && in01(0, 128)) s.music = static_cast<int>(v);
		else if (key == "sfx" && in01(0, 128)) s.sfx = static_cast<int>(v);
		else if (key == "fullscreen" && in01(0, 1)) s.fullscreen = v != 0;
		else if (key == "intro_seen" && in01(0, 1)) s.introSeen = v != 0;
		else if (key == "mode" && in01(0, 2)) s.modeIndex = static_cast<int>(v);
		else if (key == "unlocked" && in01(1, cfg::kMaxWorlds)) s.unlocked = static_cast<int>(v);
		else {
			for (int w = 0; w < cfg::kMaxWorlds; ++w) {
				for (int m = 0; m < 3; ++m) {
					if (key == bestKey(w, m) && in01(0, 0xFFFFFFFFLL)) s.best[w][m] = static_cast<Uint32>(v);
				}
			}
		}
	}
	s.difficulty = cfg::kLives[s.modeIndex];
}

void saveSettings(const Settings& s) {
	const std::filesystem::path path = savePath();
	std::filesystem::path tmp = path;
	tmp += ".tmp";
	{
		std::ofstream out(tmp, std::ios::trunc);
		out << "# Jungle Run save file\n"
			<< "music " << s.music << "\nsfx " << s.sfx << "\nfullscreen " << (s.fullscreen ? 1 : 0)
			<< "\nintro_seen " << (s.introSeen ? 1 : 0) << "\nmode " << s.modeIndex << "\nunlocked " << s.unlocked << "\n";
		for (int w = 0; w < cfg::kMaxWorlds; ++w) {
			for (int m = 0; m < 3; ++m) {
				if (s.best[w][m]) out << bestKey(w, m) << " " << s.best[w][m] << "\n";
			}
		}
		out.close();  // flush first, so a failed write never replaces the good file
		if (!out) {
			SDL_Log("save: cannot write %s", tmp.u8string().c_str());
			return;
		}
	}
	std::error_code ec;
	std::filesystem::rename(tmp, path, ec);
	if (ec) SDL_Log("save: cannot replace %s: %s", path.u8string().c_str(), ec.message().c_str());
}
}
