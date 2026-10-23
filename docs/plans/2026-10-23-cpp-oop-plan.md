# Jungle Run — Kế hoạch triển khai chuyển C sang C++ (OOP)

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Thay toàn bộ code C trong `src/` bằng C++17 hướng đối tượng (entity đa hình, scene, RAII, level từ file text) mà gameplay, hình và luồng màn giống hệt bản C.

**Architecture:** GĐ1 dựng model gameplay C++ (`game/`) chạy song song với code C trong cùng project, xuất dữ liệu level ra `Resource/Levels/`, và một exe kiểm tra song song so từng frame giữa C và C++. GĐ2 thay scene/khung ứng dụng sang C++ (`Game`, `Scene`, `Button`) và xóa toàn bộ `.c`. GĐ3 tài liệu.

**Tech Stack:** C++17 (MSVC v145 trên máy dev, project khai báo v143), SDL2 2.26 / SDL_image 2.6 / SDL_ttf 2.20 / SDL_mixer 2.6, Visual Studio `.vcxproj` x64, Python 3.12 (script xuất level, chỉ chạy một lần).

**Thiết kế:** [2026-10-23-cpp-oop-design.md](2026-10-23-cpp-oop-design.md) (APPROVED, quyết định #1–#35).

---

## Quy ước chung

**Lệnh build** (Git Bash):
```bash
MSB="/c/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/MSBuild/Current/Bin/MSBuild.exe"
"$MSB" "Jungle Run.vcxproj" -t:Rebuild -p:Configuration=Debug   -p:Platform=x64 -p:PlatformToolset=v145 -nologo -v:minimal > /tmp/jr-Debug.log 2>&1
"$MSB" "Jungle Run.vcxproj" -t:Rebuild -p:Configuration=Release -p:Platform=x64 -p:PlatformToolset=v145 -nologo -v:minimal > /tmp/jr-Release.log 2>&1
grep -E "Jungle Run.exe|: error " /tmp/jr-Debug.log
grep -E "\.cpp\([0-9,]+\): warning" /tmp/jr-Debug.log      # Expected: không có (C++ phải sạch ở /W4)
```

**Quy tắc code** (từ thiết kế):
- Mọi thứ C++ trong `namespace jr`. Hằng số trong `jr::cfg` dạng `kName` — không trùng macro C trong `game/defs.h` (`GRAVITY`, `WIDTH_WINDOW`…) vì GĐ1 compile chung với C.
- Không `new`/`delete`; tài nguyên SDL qua `jr::sdl::*Ptr`. Không handle RAII `static`/toàn cục.
- Không exception cho luồng điều khiển; số đọc từ file bằng `std::from_chars`.
- Thời gian luôn lấy bằng `SDL_GetTicks64()` (exe kiểm tra song song thay hàm này bằng đồng hồ giả qua `/FI`). Không dùng `SDL_GetTicks()` trong `src/game`.
- Port **nguyên văn** biểu thức số học từ C (kể cả literal `0.2` double, `-2.5f`, thứ tự cộng) — exe kiểm tra so khớp bit-for-bit.
- Tham số không dùng để không tên (tránh C4100 ở `/W4`). Tránh tham số trùng tên member (C4458): member có hậu tố `_`.
- Chuỗi trong code chỉ ASCII/tiếng Anh.

**Sai khác nhỏ so với thiết kế (được phép, không đổi hành vi):**
1. `Scene` nhận `Game&` trong constructor; `run()` không tham số.
2. Bảy class scene khai báo chung trong `scenes/Scenes.h`, mỗi class một `.cpp`.
3. `GameAssets` sở hữu mọi texture gameplay (kể cả 5 lớp nền); `Parallax` chỉ giữ trạng thái vị trí. Mỗi lớp nền nạp 1 lần (bản C nạp cùng 1 file 3 lần rồi vẽ cả 3 — hình như nhau). Không nạp các texture bản C nạp nhưng không vẽ (Platform 1/2/4/5, Lightning Level 1).
4. Nền đất là `struct Ground`/`SDL_FRect` trong `Level`, không phải `Entity` (đường va chạm riêng, #14).

---

## GĐ0 — Điều kiện bắt đầu (người dùng)

### Task 0.1: Xác nhận branch C và tạo branch mới

**Step 1:** Người dùng báo checklist runtime của branch `refactor/scene-loop-memleak` đạt (thiết kế C mục 6). **Không làm tiếp nếu chưa có xác nhận.**

**Step 2:** Merge và tạo branch (hỏi người dùng trước khi merge vào `main`):
```bash
git checkout main && git merge --no-ff refactor/scene-loop-memleak
git checkout -b refactor/cpp-oop
```
Expected: `Switched to a new branch 'refactor/cpp-oop'`.

---

## GĐ1 — Model C++ song song + dữ liệu level + kiểm tra song song

### Task 1.1: Bật C++17 trong project

**Files:** Modify `Jungle Run.vcxproj` (ItemDefinitionGroup `ClCompile` của Debug|x64 và Release|x64)

**Step 1:** Thêm vào cả hai `ClCompile`:
```xml
      <LanguageStandard>stdcpp17</LanguageStandard>
```

**Step 2:** Build Debug. Expected: thành công, không đổi gì (chưa có `.cpp`).

### Task 1.2: `src/game/Config.h`

```cpp
#pragma once
#include <SDL.h>

namespace jr::cfg {
constexpr int kWindowW = 1200, kWindowH = 800;  // gameplay / intro
constexpr int kMenuW = 960, kMenuH = 668;       // menu scenes

constexpr int kDifficultyEasy = 5, kDifficultyMedium = 3, kDifficultyHard = 1;  // also max lives
constexpr int kRequiredPotions = 5;

constexpr int kTile = 50;
constexpr int kGridRows = 16, kGridCols = 24;
constexpr int kSegmentW = kTile * kGridCols;  // 1200
constexpr int kGroundW = 336, kGroundH = 159; // "Platform 3"
constexpr int kMaxTileCode = 22;

constexpr float kGravity = 0.25f;
constexpr int kCharacterFrames = 90;  // ms per player animation frame
constexpr int kIdleW = 19, kIdleH = 32, kRunW = 21, kRunH = 33, kJumpW = 20, kJumpH = 35;
constexpr int kItemFrames = 100;      // ms per item animation frame
constexpr int kSmolGolemFrames = 100, kSmolGolemSpeed = 40, kSmolGolemDetect = 150;
constexpr int kGigaGolemFrames = 100, kGigaGolemSpeed = 75, kGigaGolemSize = 250;
constexpr int kParallaxSpeed[5] = { 1, 6, 5, 4, 3 };
}

namespace jr {
// Same as animation_smoothness() in the C version: global clock, not per-entity.
inline int animFrame(int frames, int periodMs) {
	return static_cast<int>(SDL_GetTicks64() / static_cast<Uint64>(periodMs) % static_cast<Uint64>(frames));
}
}
```

### Task 1.3: `src/core/Sdl.h` + `Sdl.cpp`

**`src/core/Sdl.h`:**
```cpp
#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <array>
#include <memory>
#include <string>

namespace jr::sdl {
enum class Kind { Texture, Font, Chunk, Music };
void track(Kind kind, int delta);  // Debug: live resource counts; Release: no-op
std::string liveCounts();          // Debug: "tex 42 font 3 wav 7 mus 2 | sdl_alloc 1234"; Release: ""

struct TextureDeleter { void operator()(SDL_Texture* p) const; };
struct FontDeleter { void operator()(TTF_Font* p) const; };
struct ChunkDeleter { void operator()(Mix_Chunk* p) const; };
struct MusicDeleter { void operator()(Mix_Music* p) const; };
struct WindowDeleter { void operator()(SDL_Window* p) const { SDL_DestroyWindow(p); } };
struct RendererDeleter { void operator()(SDL_Renderer* p) const { SDL_DestroyRenderer(p); } };

using TexturePtr = std::unique_ptr<SDL_Texture, TextureDeleter>;
using FontPtr = std::unique_ptr<TTF_Font, FontDeleter>;
using ChunkPtr = std::unique_ptr<Mix_Chunk, ChunkDeleter>;
using MusicPtr = std::unique_ptr<Mix_Music, MusicDeleter>;
using WindowPtr = std::unique_ptr<SDL_Window, WindowDeleter>;
using RendererPtr = std::unique_ptr<SDL_Renderer, RendererDeleter>;

TexturePtr loadTexture(SDL_Renderer* r, const std::string& path, bool keyBlack = false);
TexturePtr renderText(SDL_Renderer* r, TTF_Font* font, const std::string& text, SDL_Color color);
FontPtr loadFont(const std::string& path, int size);
ChunkPtr loadChunk(const std::string& path, int volume = -1);  // volume < 0: keep default
MusicPtr loadMusic(const std::string& path);
SDL_Rect sized(SDL_Texture* t, int x, int y);                // rect at (x, y) with the texture's size

template <size_t N>
void loadTextureSeq(SDL_Renderer* r, std::array<TexturePtr, N>& out, const char* fmt, bool keyBlack) {
	char path[256];
	for (size_t i = 0; i < N; ++i) {
		SDL_snprintf(path, sizeof path, fmt, static_cast<int>(i + 1));
		out[i] = loadTexture(r, path, keyBlack);
	}
}

template <size_t N>
bool allLoaded(const std::array<TexturePtr, N>& a) {
	for (const auto& t : a) {
		if (!t) return false;
	}
	return true;
}

// SDL + SDL_image/ttf/mixer init/quit.
class Session {
public:
	Session();
	~Session();
	Session(const Session&) = delete;
	Session& operator=(const Session&) = delete;
	bool ok() const { return ok_; }
private:
	bool ok_ = false;
};

// Mix_OpenAudio / Mix_CloseAudio. Failure is not fatal: the game runs silent.
class AudioDevice {
public:
	AudioDevice();
	~AudioDevice();
	AudioDevice(const AudioDevice&) = delete;
	AudioDevice& operator=(const AudioDevice&) = delete;
private:
	bool open_ = false;
};
}
```

**`src/core/Sdl.cpp`:**
```cpp
#include "core/Sdl.h"

namespace jr::sdl {
#ifdef _DEBUG
namespace {
std::array<int, 4> live{};
}
void track(Kind kind, int delta) { live[static_cast<size_t>(kind)] += delta; }
std::string liveCounts() {
	char buf[128];
	SDL_snprintf(buf, sizeof buf, "tex %d font %d wav %d mus %d | sdl_alloc %d",
		live[0], live[1], live[2], live[3], SDL_GetNumAllocations());
	return buf;
}
#else
void track(Kind, int) {}
std::string liveCounts() { return {}; }
#endif

void TextureDeleter::operator()(SDL_Texture* p) const { SDL_DestroyTexture(p); track(Kind::Texture, -1); }
void FontDeleter::operator()(TTF_Font* p) const { TTF_CloseFont(p); track(Kind::Font, -1); }
void ChunkDeleter::operator()(Mix_Chunk* p) const { Mix_FreeChunk(p); track(Kind::Chunk, -1); }
void MusicDeleter::operator()(Mix_Music* p) const { Mix_FreeMusic(p); track(Kind::Music, -1); }

namespace {
TexturePtr fromSurface(SDL_Renderer* r, SDL_Surface* s, const std::string& what, bool keyBlack) {
	if (!s) {
		SDL_Log("load fail: %s: %s", what.c_str(), SDL_GetError());
		return nullptr;
	}
	if (keyBlack) {
		SDL_SetColorKey(s, SDL_TRUE, SDL_MapRGB(s->format, 0, 0, 0));
	}
	SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
	SDL_FreeSurface(s);
	if (!t) {
		SDL_Log("texture fail: %s: %s", what.c_str(), SDL_GetError());
		return nullptr;
	}
	track(Kind::Texture, +1);
	return TexturePtr(t);
}
}

TexturePtr loadTexture(SDL_Renderer* r, const std::string& path, bool keyBlack) {
	return fromSurface(r, IMG_Load(path.c_str()), path, keyBlack);
}

TexturePtr renderText(SDL_Renderer* r, TTF_Font* font, const std::string& text, SDL_Color color) {
	return fromSurface(r, font ? TTF_RenderText_Solid(font, text.c_str(), color) : nullptr, text, false);
}

FontPtr loadFont(const std::string& path, int size) {
	TTF_Font* f = TTF_OpenFont(path.c_str(), size);
	if (!f) {
		SDL_Log("font fail: %s: %s", path.c_str(), SDL_GetError());
		return nullptr;
	}
	track(Kind::Font, +1);
	return FontPtr(f);
}

ChunkPtr loadChunk(const std::string& path, int volume) {
	Mix_Chunk* c = Mix_LoadWAV(path.c_str());
	if (!c) {
		SDL_Log("sound fail: %s: %s", path.c_str(), SDL_GetError());
		return nullptr;
	}
	if (volume >= 0) {
		Mix_VolumeChunk(c, volume);
	}
	track(Kind::Chunk, +1);
	return ChunkPtr(c);
}

MusicPtr loadMusic(const std::string& path) {
	Mix_Music* m = Mix_LoadMUS(path.c_str());
	if (!m) {
		SDL_Log("music fail: %s: %s", path.c_str(), SDL_GetError());
		return nullptr;
	}
	track(Kind::Music, +1);
	return MusicPtr(m);
}

SDL_Rect sized(SDL_Texture* t, int x, int y) {
	SDL_Rect r{ x, y, 0, 0 };
	SDL_QueryTexture(t, nullptr, nullptr, &r.w, &r.h);
	return r;
}

Session::Session() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
		SDL_Log("SDL_Init: %s", SDL_GetError());
		return;
	}
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	Mix_Init(MIX_INIT_MP3);
	ok_ = true;
}

Session::~Session() {
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

AudioDevice::AudioDevice() {
	open_ = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048) == 0;
	if (!open_) {
		SDL_Log("audio: %s", SDL_GetError());
	}
}

AudioDevice::~AudioDevice() {
	if (open_) {
		Mix_HaltMusic();
		Mix_CloseAudio();
	}
}
}
```

### Task 1.4: Xuất dữ liệu level + `course.txt`

**Files:** Create `tools/export_levels.py`, `Resource/Levels/course.txt`; generated `Resource/Levels/lv00.txt` … `lv15.txt`

**Step 1: `Resource/Levels/course.txt`**
```
# Jungle Run course: segments placed left to right, x cursor starts at 0.
#   ground N        N ground platforms, 336 px each
#   map A [B ...]   one 1200 px segment per grid file (Resource/Levels/A.txt)
#   onkey A B       when the key (tile 21) is picked up, segment A is rebuilt from grid B at the same x
#   finish X        reaching x >= X (pixels from the left edge of the course) ends the run
ground 3
map lv00
ground 2
map lv01
ground 2
map lv02
ground 2
map lv03
ground 2
map lv04
ground 2
map lv05 lv06
ground 2
map lv07 lv08
ground 2
map lv09
ground 2
map lv10
ground 2
map lv11 lv12
ground 2
map lv13 lv15
ground 27
onkey lv15 lv14
finish 25500
# course length: 50 x 336 + 15 x 1200 = 34800 px
```

**Step 2: `tools/export_levels.py`**
```python
"""One-off: export src/game/levels.c grids to Resource/Levels/lvNN.txt, then verify.

Run from the repo root:  python tools/export_levels.py
Exit code 0 and "OK" lines on success.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LEVELS_C = ROOT / "src" / "game" / "levels.c"
OUT = ROOT / "Resource" / "Levels"
ROWS, COLS = 16, 24

LEGEND = """\
# Jungle Run level grid: 16 rows x 24 numbers. Row 0 is the top; 24 columns = one 1200 px screen; each cell 50 px.
# 0 empty | 1 brick | 2 slope | 3 wide platform (3 cells)
# 4 fire trap lv1 | 5 fire trap lv2 | 6 fire trap lv3 (draws 1 cell above)
# 7 ceiling spike, upside down | 20 ceiling spike
# 8 / 9 / 10 falling ceiling slow / medium / fast (draws 2 cells above; solid while down)
# 11 floor saw | 12 ceiling saw
# 13 first aid kit (+1 life) | 14 health potion (5 = good ending) | 21 key (triggers 'onkey' in course.txt)
# 15 / 16 / 17 / 18 lightning super slow / slow / medium / fast (draws 2 cells above)
# 19 small golem (chases the player) | 22 sparkle (decoration)
# Do not put 8-10 or 15-18 in rows 0-1, or 6 in row 0: they would be drawn off screen.
"""


def read_c_grids():
    text = LEVELS_C.read_text(encoding="utf-8")
    grids = {}
    for name, body in re.findall(r"int (lv\d+)\[MAP_HEIGHT\]\[MAP_WIDTH\] = \{(.*?)\};", text, re.S):
        nums = [int(n) for n in re.findall(r"-?\d+", body)]
        assert len(nums) == ROWS * COLS, (name, len(nums))
        grids["lv%02d" % int(name[2:])] = [nums[r * COLS:(r + 1) * COLS] for r in range(ROWS)]
    assert len(grids) == 16, len(grids)
    return grids


def write_grids(grids):
    OUT.mkdir(parents=True, exist_ok=True)
    for name, rows in grids.items():
        lines = [LEGEND] + [" ".join("%2d" % v for v in row) for row in rows]
        (OUT / (name + ".txt")).write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\r\n")


def read_txt_grid(path):
    rows = []
    for line in path.read_text(encoding="utf-8-sig").splitlines():
        data = line.split("#", 1)[0].split()
        if data:
            rows.append([int(v) for v in data])
    return rows


def c_course():
    """Python port of drawMap_SingleplayerMode (src/game/map.c)."""
    w3, w = 336.0, 1200.0
    single = {3: "lv00", 5: "lv01", 7: "lv02", 9: "lv03", 11: "lv04", 17: "lv09", 19: "lv10"}
    double = {13: ("lv05", "lv06"), 15: ("lv07", "lv08"), 21: ("lv11", "lv12"), 23: ("lv13", "lv15")}
    ledges, maps = [0.0], []
    for i in range(1, 50):
        prev, add = ledges[i - 1], 0.0
        if i in single:
            add = w
            maps.append((single[i], prev + w3))
        elif i in double:
            add = 2 * w
            maps.append((double[i][0], prev + w3))
            maps.append((double[i][1], prev + w3 + w))
        ledges.append(prev + w3 + add)
    return ledges, maps


def txt_course():
    """Same cursor rules as jr::loadCourse (src/game/LevelLoader.cpp)."""
    cursor, ledges, maps, finish, onkey = 0.0, [], [], None, None
    for line in (OUT / "course.txt").read_text(encoding="utf-8-sig").splitlines():
        t = line.split("#", 1)[0].split()
        if not t:
            continue
        if t[0] == "ground":
            for _ in range(int(t[1])):
                ledges.append(cursor)
                cursor += 336.0
        elif t[0] == "map":
            for name in t[1:]:
                maps.append((name, cursor))
                cursor += 1200.0
        elif t[0] == "onkey":
            onkey = (t[1], t[2])
        elif t[0] == "finish":
            finish = int(t[1])
    return ledges, maps, finish, onkey, cursor


def main():
    grids = read_c_grids()
    write_grids(grids)
    bad = [n for n, rows in grids.items() if read_txt_grid(OUT / (n + ".txt")) != rows]
    print("grids: %d/16 OK" % (16 - len(bad)) + ("" if not bad else "  MISMATCH: " + ", ".join(bad)))

    c_ledges, c_maps = c_course()
    t_ledges, t_maps, finish, onkey, length = txt_course()
    ok = c_ledges == t_ledges and c_maps == t_maps and finish == 25500 and onkey == ("lv15", "lv14")
    print("course: %s (%d grounds, %d maps, length %d)" % ("OK" if ok else "MISMATCH", len(t_ledges), len(t_maps), length))
    if not ok:
        print("C   :", c_ledges, c_maps)
        print("TXT :", t_ledges, t_maps, finish, onkey)
    sys.exit(0 if ok and not bad else 1)


if __name__ == "__main__":
    main()
```

**Step 3: Chạy**
```bash
python tools/export_levels.py
```
Expected:
```
grids: 16/16 OK
course: OK (50 grounds, 15 maps, length 34800)
```

**Step 4: Commit**
```bash
git add tools/export_levels.py Resource/Levels "Jungle Run.vcxproj"
git commit -m "feat(levels): export level grids and course layout to Resource/Levels"
```

### Task 1.5: `src/game/LevelLoader.h` + `.cpp`

**`LevelLoader.h`:**
```cpp
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
```

**`LevelLoader.cpp`:**
```cpp
#include "game/LevelLoader.h"
#include <charconv>
#include <fstream>
#include <iterator>
#include <sstream>

namespace jr {
namespace {
constexpr int kKeyTile = 21;

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
```

### Task 1.6: Entity + các class con + factory + GameAssets

**`src/game/GameAssets.h`:**
```cpp
#pragma once
#include "core/Sdl.h"
#include "game/Entity.h"

namespace jr {
enum class TextureId { Tileset, Ground, Fire1, Fire2, Fire3, Ceiling, Saw, Lightning2, Items, Golem, Sparkle, Count };

// Owns every gameplay texture, font and sound. Entities hold raw pointers borrowed from here,
// so GameplayScene declares GameAssets before World.
class GameAssets {
public:
	bool load(SDL_Renderer* r);  // false if an image or font is missing; sounds are optional
	SDL_Texture* tex(TextureId id) const { return tex_[static_cast<size_t>(id)].get(); }
	SDL_Texture* idle(int i) const { return idle_[i].get(); }
	SDL_Texture* run(int i) const { return run_[i].get(); }
	SDL_Texture* jump(int i) const { return jump_[i].get(); }
	SDL_Texture* background(int i) const { return background_[i].get(); }
	TTF_Font* hudFont() const { return hudFont_.get(); }
	TTF_Font* overFont() const { return overFont_.get(); }
	Mix_Chunk* hurt(HurtSound s) const;
	Mix_Chunk* itemPickUp() const { return itemPickUp_.get(); }
	Mix_Chunk* landing() const { return landing_.get(); }
	Mix_Chunk* jumpSound() const { return jumpSound_.get(); }
	Mix_Music* inGameMusic() const { return inGame_.get(); }
	Mix_Music* endingMusic(int i) const { return ending_[i].get(); }
private:
	std::array<sdl::TexturePtr, static_cast<size_t>(TextureId::Count)> tex_;
	std::array<sdl::TexturePtr, 12> idle_;
	std::array<sdl::TexturePtr, 8> run_;
	std::array<sdl::TexturePtr, 4> jump_;
	std::array<sdl::TexturePtr, 5> background_;
	sdl::FontPtr hudFont_, overFont_;
	sdl::ChunkPtr damage_, electric_, spike_, itemPickUp_, landing_, jumpSound_;
	sdl::MusicPtr inGame_;
	std::array<sdl::MusicPtr, 3> ending_;
};
}
```

**`src/game/GameAssets.cpp`:**
```cpp
#include "game/GameAssets.h"

namespace jr {
namespace {
const char* const kTraps = "Resource\\Foozle_2DTR0001_Pixel_Trap_Pack\\";
std::string trap(const char* rest) { return std::string(kTraps) + rest; }
}

bool GameAssets::load(SDL_Renderer* r) {
	sdl::loadTextureSeq(r, idle_, "Resource\\Character\\Idle\\Idle_%d.png", true);
	sdl::loadTextureSeq(r, run_, "Resource\\Character\\Run\\Run_%d.png", true);
	sdl::loadTextureSeq(r, jump_, "Resource\\Character\\Jump\\Jump_%d.png", true);
	sdl::loadTextureSeq(r, background_, "Resource\\Jungle Asset Pack\\parallax background\\plx-%d.png", false);
	auto set = [&](TextureId id, const std::string& path, bool keyBlack) {
		tex_[static_cast<size_t>(id)] = sdl::loadTexture(r, path, keyBlack);
	};
	set(TextureId::Tileset, "Resource\\Jungle Asset Pack\\jungle tileset\\jungle tileset.png", false);
	set(TextureId::Ground, "Resource\\Some Bullshit Platform\\Platform 3.png", true);
	set(TextureId::Fire1, trap("Fire Trap\\PNGs\\Fire Trap - Level 1.png"), true);
	set(TextureId::Fire2, trap("Fire Trap\\PNGs\\Fire Trap - Level 2.png"), true);
	set(TextureId::Fire3, trap("Fire Trap\\PNGs\\Fire Trap - Level 3.png"), true);
	set(TextureId::Ceiling, trap("Ceiling Trap\\PNGs\\Ceiling Trap - Level 1.png"), true);
	set(TextureId::Saw, trap("Saw Trap\\PNGs\\Saw Trap - Level 1.png"), true);
	set(TextureId::Lightning2, trap("Lightning Trap\\PNGs\\Lightning Trap - Level 2.png"), true);
	set(TextureId::Items, "Resource\\platformer items\\animated_items.png", true);
	set(TextureId::Golem, "Resource\\Mecha-stone Golem 0.1\\PNG sheet\\Character_sheet.png", true);
	set(TextureId::Sparkle, "Resource\\sparkle\\sparkle.png", true);

	hudFont_ = sdl::loadFont("Resource\\Fonts\\crazy-pixel.ttf", 48);
	overFont_ = sdl::loadFont("Resource\\Fonts\\Kanit-Light.ttf", 108);

	damage_ = sdl::loadChunk("Resource\\Sound\\Sound Effects\\DamageSound.wav", 64);
	electric_ = sdl::loadChunk("Resource\\Sound\\Sound Effects\\ElectricHurtSound.wav", 64);
	itemPickUp_ = sdl::loadChunk("Resource\\Sound\\Sound Effects\\ItemPickupSound.wav", 96);
	jumpSound_ = sdl::loadChunk("Resource\\Sound\\Sound Effects\\jump.wav", 16);
	landing_ = sdl::loadChunk("Resource\\Sound\\Sound Effects\\LandingSound.wav", 16);
	spike_ = sdl::loadChunk("Resource\\Sound\\Sound Effects\\SpikeSound.wav", 32);
	inGame_ = sdl::loadMusic("Resource\\Sound\\Soundtrack\\In Game 3.mp3");
	ending_[0] = sdl::loadMusic("Resource\\Sound\\Soundtrack\\Bad Ending.mp3");
	ending_[1] = sdl::loadMusic("Resource\\Sound\\Soundtrack\\True Ending.mp3");
	ending_[2] = sdl::loadMusic("Resource\\Sound\\Soundtrack\\Good Ending.mp3");
	Mix_VolumeMusic(32);

	return hudFont_ && overFont_ && sdl::allLoaded(tex_) && sdl::allLoaded(idle_) && sdl::allLoaded(run_) &&
		sdl::allLoaded(jump_) && sdl::allLoaded(background_);
}

Mix_Chunk* GameAssets::hurt(HurtSound s) const {
	switch (s) {
	case HurtSound::Spike: return spike_.get();
	case HurtSound::Electric: return electric_.get();
	default: return damage_.get();
	}
}
}
```

**`src/game/Entity.h`:**
```cpp
#pragma once
#include "game/Config.h"

namespace jr {
class GameAssets;
class Player;
class World;

enum class HurtSound { Damage, Spike, Electric };

class Entity {
public:
	virtual ~Entity() = default;
	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	virtual void preCollide(const Player&, float) {}       // SmolGolem moves here, before its own hit test
	virtual void onTouch(Player&, World&) {}               // Pickup effects
	virtual bool isSolid() const { return false; }
	virtual bool isLethal() const { return false; }
	virtual HurtSound hurtSound() const { return HurtSound::Damage; }
	virtual bool grantsKey() const { return false; }       // a Key that has been picked up
	virtual bool isTaken() const { return false; }         // a Pickup that has been picked up
	virtual void animate(Uint64) {}                        // render phase: Trap updates its stored flags
	virtual void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const = 0;

	const SDL_FRect& bounds() const { return box_; }

protected:
	explicit Entity(SDL_FRect box) : box_(box) {}
	// Same truncation as drawTilesMap(): { scrollX + x, y, w, h } converted to int.
	SDL_Rect dst(float scrollX) const {
		return { static_cast<int>(scrollX + box_.x), static_cast<int>(box_.y), static_cast<int>(box_.w), static_cast<int>(box_.h) };
	}
	SDL_FRect box_;
};

// Same truncation as rectA/rectB in collision_detect_map().
inline SDL_Rect toIntRect(const SDL_FRect& b) {
	return { static_cast<int>(b.x), static_cast<int>(b.y), static_cast<int>(b.w), static_cast<int>(b.h) };
}
}
```

**`src/game/Tile.h`** (header-only, nhỏ):
```cpp
#pragma once
#include "game/Entity.h"
#include "game/GameAssets.h"

namespace jr {
class Tile final : public Entity {
public:
	Tile(SDL_FRect box, SDL_Rect src) : Entity(box), src_(src) {}
	bool isSolid() const override { return true; }
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const override {
		const SDL_Rect d = dst(scrollX);
		SDL_RenderCopyEx(r, a.tex(TextureId::Tileset), &src_, &d, 0, nullptr, SDL_FLIP_NONE);
	}
private:
	SDL_Rect src_;
};
}
```

**`src/game/Trap.h`:**
```cpp
#pragma once
#include "game/Entity.h"
#include "game/GameAssets.h"

namespace jr {
struct TrapSpec {
	TextureId tex;
	int frames, periodMs;    // frame i = nowMs / periodMs % frames
	SDL_Rect src;            // source rect of frame 0
	int stepX;               // frame i source x = src.x + i * stepX
	double angle;
	Uint32 lethalMask;       // bit i: frame i hurts
	Uint32 solidOnMask;      // bit i: frame i blocks
	Uint32 solidOffMask;     // bit i: frame i does not block; frames in neither mask keep the previous state
	HurtSound sound;
	int yRows, hRows;        // bounds: y += yRows * 50, h = hRows * 50
	bool initialLethal;      // codes 7 and 20 start lethal (loadMap)
};

// Lethal/solid are STORED and updated in animate() (render phase); collision reads the
// previous frame's values, like the C version (drawMap runs after collision).
class Trap final : public Entity {
public:
	Trap(SDL_FRect box, const TrapSpec& spec) : Entity(box), spec_(spec), lethal_(spec.initialLethal) {}
	bool isSolid() const override { return solid_; }
	bool isLethal() const override { return lethal_; }
	HurtSound hurtSound() const override { return spec_.sound; }
	void animate(Uint64 nowMs) override {
		frame_ = static_cast<int>(nowMs / static_cast<Uint64>(spec_.periodMs) % static_cast<Uint64>(spec_.frames));
		const Uint32 bit = 1u << frame_;
		lethal_ = (spec_.lethalMask & bit) != 0;
		if (spec_.solidOnMask & bit) solid_ = true;
		else if (spec_.solidOffMask & bit) solid_ = false;
	}
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const override {
		SDL_Rect src = spec_.src;
		src.x += frame_ * spec_.stepX;
		const SDL_Rect d = dst(scrollX);
		SDL_RenderCopyEx(r, a.tex(spec_.tex), &src, &d, spec_.angle, nullptr, SDL_FLIP_NONE);
	}
private:
	const TrapSpec& spec_;  // points into the static table in EntityFactory.cpp
	int frame_ = 0;
	bool lethal_;
	bool solid_ = false;
};
}
```

**`src/game/Pickup.h` + `Pickup.cpp`:**
```cpp
// Pickup.h
#pragma once
#include "game/Entity.h"

namespace jr {
enum class PickupKind { FirstAid, Potion, Key };

class Pickup final : public Entity {
public:
	Pickup(SDL_FRect box, PickupKind kind) : Entity(box), kind_(kind) {}
	void onTouch(Player& p, World& w) override;
	bool grantsKey() const override { return kind_ == PickupKind::Key && taken_; }
	bool isTaken() const override { return taken_; }
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const override;
private:
	PickupKind kind_;
	bool taken_ = false;
};
}
```
```cpp
// Pickup.cpp
#include "game/Pickup.h"
#include "game/GameAssets.h"
#include "game/World.h"

namespace jr {
void Pickup::onTouch(Player& p, World& w) {
	if (taken_) return;
	taken_ = true;
	if (kind_ == PickupKind::FirstAid) p.heal(w.difficulty());
	else if (kind_ == PickupKind::Potion) w.addPotion();
	Mix_PlayChannel(-1, w.assets().itemPickUp(), 0);
}

void Pickup::draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const {
	if (taken_) return;
	const int frames = kind_ == PickupKind::FirstAid ? 7 : 8;
	const int row = kind_ == PickupKind::FirstAid ? 4 : kind_ == PickupKind::Potion ? 9 : 13;
	const SDL_Rect src{ animFrame(frames, cfg::kItemFrames) * 32, row * 32, 32, 32 };
	const SDL_Rect d = dst(scrollX);
	SDL_RenderCopyEx(r, a.tex(TextureId::Items), &src, &d, 0, nullptr, SDL_FLIP_NONE);
}
}
```

**`src/game/SmolGolem.h` + `.cpp`:**
```cpp
// SmolGolem.h
#pragma once
#include "game/Entity.h"

namespace jr {
class SmolGolem final : public Entity {
public:
	explicit SmolGolem(SDL_FRect box) : Entity(box) {}
	void preCollide(const Player& p, float dt) override;  // = detectPlayer + updateGolem
	bool isLethal() const override { return true; }
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const override;
private:
	float dx_ = 0;
	int flip_ = 0;
};
}
```
```cpp
// SmolGolem.cpp
#include "game/SmolGolem.h"
#include "game/GameAssets.h"
#include "game/Player.h"
#include <cmath>

namespace jr {
void SmolGolem::preCollide(const Player& p, float dt) {
	float px, py, pw, ph;
	p.hitInfo(px, py, pw, ph);
	const float bw = static_cast<float>(cfg::kTile), bx = box_.x, by = box_.y;
	if (px + pw >= bx - cfg::kSmolGolemDetect && px <= bx + bw + cfg::kSmolGolemDetect &&
		std::fabs(static_cast<double>(by - py)) <= 2 * cfg::kTile) {
		if (px < bx + (bw / 2)) {
			dx_ += -cfg::kSmolGolemSpeed;
			flip_ = 1;
		}
		else if (px > bx + (bw / 2)) {
			dx_ += cfg::kSmolGolemSpeed;
			flip_ = 0;
		}
	}
	else {
		dx_ = 0.f;
	}
	if (dx_ > 120) dx_ = 120;
	if (dx_ < -120) dx_ = -120;
	box_.x += dx_ * dt;
}

void SmolGolem::draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const {
	const SDL_Rect src{ 24 + animFrame(8, cfg::kSmolGolemFrames) * 100, 124, 50, 50 };
	const SDL_Rect d = dst(scrollX);
	SDL_RenderCopyEx(r, a.tex(TextureId::Golem), &src, &d, 0, nullptr, flip_ ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}
}
```

**`src/game/Decoration.h`** (header-only):
```cpp
#pragma once
#include "game/Entity.h"
#include "game/GameAssets.h"

namespace jr {
class Decoration final : public Entity {  // sparkle, code 22
public:
	explicit Decoration(SDL_FRect box) : Entity(box) {}
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const override {
		const int i = animFrame(16, cfg::kItemFrames);
		const SDL_Rect src{ i % 4 * 32, i / 4 * 32, 32, 32 };
		const SDL_Rect d = dst(scrollX);
		SDL_RenderCopyEx(r, a.tex(TextureId::Sparkle), &src, &d, 0, nullptr, SDL_FLIP_NONE);
	}
};
}
```

**`src/game/EntityFactory.h` + `.cpp`:**
```cpp
// EntityFactory.h
#pragma once
#include "game/Entity.h"
#include <memory>

namespace jr {
// Tile code (see Resource/Levels legend) at cell top-left (x, y) -> entity; code 0 -> nullptr.
std::unique_ptr<Entity> makeEntity(int code, float x, float y);
}
```
```cpp
// EntityFactory.cpp
#include "game/EntityFactory.h"
#include "game/Decoration.h"
#include "game/Pickup.h"
#include "game/SmolGolem.h"
#include "game/Tile.h"
#include "game/Trap.h"
#include <initializer_list>

namespace jr {
namespace {
constexpr Uint32 bits(std::initializer_list<int> frames) {
	Uint32 m = 0;
	for (int f : frames) m |= 1u << f;
	return m;
}
constexpr Uint32 range(int first, int last) {
	Uint32 m = 0;
	for (int f = first; f <= last; ++f) m |= 1u << f;
	return m;
}
constexpr Uint32 all(int frames) { return range(0, frames - 1); }

struct TrapRow {
	int code;
	TrapSpec spec;
};

// Values from loadMap() + drawMap() in the C version (src/game/map.c).
const TrapRow kTraps[] = {
	{ 4, { TextureId::Fire1, 9, 200, { 8, 31, 16, 33 }, 32, 0, bits({ 4, 5 }), 0, 0, HurtSound::Damage, 0, 1, false } },
	{ 5, { TextureId::Fire2, 9, 175, { 3, 25, 26, 40 }, 32, 0, bits({ 4, 5 }), 0, 0, HurtSound::Damage, 0, 1, false } },
	{ 6, { TextureId::Fire3, 9, 250, { 3, 0, 26, 64 }, 32, 0, range(4, 8), 0, 0, HurtSound::Damage, -1, 2, false } },
	{ 7, { TextureId::Ceiling, 6, 250, { 16, 0, 33, 20 }, 0, 180, bits({ 0 }), 0, 0, HurtSound::Damage, 0, 1, true } },
	{ 8, { TextureId::Ceiling, 14, 250, { 15, 0, 34, 64 }, 64, 0, bits({ 1 }), range(2, 7), all(14) & ~range(1, 7), HurtSound::Spike, -2, 3, false } },
	{ 9, { TextureId::Ceiling, 14, 150, { 15, 0, 34, 64 }, 64, 0, bits({ 1 }), range(2, 7), all(14) & ~range(1, 7), HurtSound::Spike, -2, 3, false } },
	{ 10, { TextureId::Ceiling, 14, 75, { 15, 0, 34, 64 }, 64, 0, bits({ 1 }), range(2, 7), all(14) & ~range(1, 7), HurtSound::Spike, -2, 3, false } },
	{ 11, { TextureId::Saw, 16, 150, { 13, 0, 39, 35 }, 64, 180, all(16) & ~bits({ 0, 7, 14, 15 }), 0, 0, HurtSound::Damage, 0, 1, false } },
	{ 12, { TextureId::Saw, 16, 150, { 13, 0, 39, 35 }, 64, 0, all(16) & ~bits({ 0, 6, 7, 13, 14, 15 }), 0, 0, HurtSound::Damage, 0, 1, false } },
	{ 15, { TextureId::Lightning2, 12, 150, { 31, 0, 34, 96 }, 96, 0, range(3, 10), 0, 0, HurtSound::Electric, -2, 3, false } },
	{ 16, { TextureId::Lightning2, 12, 130, { 31, 0, 34, 96 }, 96, 0, range(3, 10), 0, 0, HurtSound::Electric, -2, 3, false } },
	{ 17, { TextureId::Lightning2, 12, 110, { 31, 0, 34, 96 }, 96, 0, range(3, 10), 0, 0, HurtSound::Electric, -2, 3, false } },
	{ 18, { TextureId::Lightning2, 12, 100, { 31, 0, 34, 96 }, 96, 0, range(3, 10), 0, 0, HurtSound::Electric, -2, 3, false } },
	{ 20, { TextureId::Ceiling, 6, 250, { 16, 0, 33, 20 }, 0, 0, bits({ 0 }), 0, 0, HurtSound::Damage, 0, 1, true } },
};
}

std::unique_ptr<Entity> makeEntity(int code, float x, float y) {
	const float t = static_cast<float>(cfg::kTile);
	const SDL_FRect cell{ x, y, t, t };
	switch (code) {
	case 0: return nullptr;
	case 1: return std::make_unique<Tile>(cell, SDL_Rect{ 63, 128, 50, 50 });
	case 2: return std::make_unique<Tile>(cell, SDL_Rect{ 463, 144, 50, 50 });
	case 3: return std::make_unique<Tile>(SDL_FRect{ x, y, 3 * t, t }, SDL_Rect{ 320, 144, 113, 49 });
	case 13: return std::make_unique<Pickup>(cell, PickupKind::FirstAid);
	case 14: return std::make_unique<Pickup>(cell, PickupKind::Potion);
	case 19: return std::make_unique<SmolGolem>(cell);
	case 21: return std::make_unique<Pickup>(cell, PickupKind::Key);
	case 22: return std::make_unique<Decoration>(cell);
	default: break;
	}
	for (const TrapRow& row : kTraps) {
		if (row.code == code) {
			const TrapSpec& s = row.spec;
			return std::make_unique<Trap>(SDL_FRect{ x, y + s.yRows * t, t, s.hRows * t }, s);
		}
	}
	return nullptr;  // unreachable: LevelLoader rejects codes outside 0-22
}
}
```

### Task 1.7: Player, GigaGolem, Parallax, Hud

**`src/game/Player.h` + `.cpp`** — đặc điểm 4.5b của thiết kế được giữ nguyên:
```cpp
// Player.h
#pragma once
#include "game/Config.h"
#include <SDL_mixer.h>

namespace jr {
class GameAssets;

struct InputState { bool left = false, right = false, down = false; };

struct PlayerState {
	float x = 500, y = 220, dx = 0, dy = 0;
	int lives = 0;
	int isTakenDamage = 0, isImmortal = 0;
	float immortalStartTime = 0;
	int onLedge = 0, flip = 0;
	int status = 0;  // 0 idle, 1 run, 2 jump, 3 mid air, 4 falling
};

class Player {
public:
	void reset(int lives);
	void input(const InputState& in);                                // = movement part of handle_input()
	void jump();                                                    // only from a ledge; onLedge is cleared only here
	void hitInfo(float& px, float& py, float& pw, float& ph) const;  // = load_player_info(): running uses IDLE size
	SDL_Rect hitRect() const;
	void resolveAgainst(const SDL_FRect& b, Mix_Chunk* landing);     // = collision_correction(), verbatim
	void hurt(Mix_Chunk* sound);                                     // caller checks lethal && !immortal
	void clearHurt() { st_.isTakenDamage = 0; }
	void heal(int maxLives);
	void updateImmortal(float nowSec);                               // = immortal_events()
	void integrate(Mix_Chunk* jumpSound);                            // move, gravity, airborne status
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const;
	void teleport(float x, float y) { st_.x = x; st_.y = y; }       // used by tools/parity only
	const PlayerState& state() const { return st_; }
private:
	PlayerState st_;
};
}
```
```cpp
// Player.cpp
#include "game/Player.h"
#include "game/GameAssets.h"
#include <cmath>

namespace jr {
void Player::reset(int lives) {
	st_ = PlayerState{};
	st_.lives = lives;
}

void Player::input(const InputState& in) {
	if (in.left) {
		st_.status = 1;
		st_.flip = 1;
		st_.dx -= 0.3f;
		if (st_.dx < -2.5f) st_.dx = -2.5f;
	}
	else if (in.right) {
		st_.status = 1;
		st_.flip = 0;
		st_.dx += 0.3f;
		if (st_.dx > 2.5f) st_.dx = 2.5f;
	}
	else if (in.down) {
		st_.dy += 0.3f;
	}
	else {
		st_.status = 0;
		st_.flip = 0;
		st_.dx *= 0.8f;
		if (std::fabs(st_.dx) < 0.1f) st_.dx = 0;
	}
}

void Player::jump() {
	if (st_.onLedge) {
		st_.onLedge = 0;
		st_.dy = -9;
	}
}

void Player::hitInfo(float& px, float& py, float& pw, float& ph) const {
	pw = cfg::kIdleW * 2, ph = cfg::kIdleH * 2;
	if (st_.status == 2 || st_.status == 3 || st_.status == 4) {
		pw = cfg::kJumpW * 2, ph = cfg::kJumpH * 2;
	}
	px = st_.x, py = st_.y;
}

SDL_Rect Player::hitRect() const {  // = rectA in collision_detect_map (float -> int truncation)
	float px, py, pw, ph;
	hitInfo(px, py, pw, ph);
	return { static_cast<int>(px), static_cast<int>(py), static_cast<int>(pw), static_cast<int>(ph) };
}

void Player::resolveAgainst(const SDL_FRect& b, Mix_Chunk* landing) {
	float px, py, pw, ph;
	hitInfo(px, py, pw, ph);
	const float bx = b.x, by = b.y, bw = b.w, bh = b.h;
	if (py + 0.2 * ph > by && py < by + bh) {
		if (px < (bx + bw) && (px + pw) > (bx + bw) && st_.dx < 0) {
			st_.x = bx + bw;
			st_.dx = 0;
		}
		else if ((px + pw) > bx && px < bx && st_.dx > 0) {
			st_.x = bx - pw;
			st_.dx = 0;
		}
	}
	if (px + pw / 2 > bx && px + pw / 2 < bx + bw) {
		if (py < (by + bh) && py > by && st_.dy < 0) {
			st_.y = by + bh;
			st_.dy = 0;
		}
	}
	if (px + pw > bx && px < bx + bw) {
		if (py + ph > by && py < by && st_.dy > 0) {
			st_.y = by - ph;
			if (!st_.onLedge) Mix_PlayChannel(-1, landing, 0);
			st_.dy = 0;
			st_.onLedge = 1;
		}
	}
}

void Player::hurt(Mix_Chunk* sound) {
	st_.isTakenDamage = 1;
	st_.lives--;
	st_.isImmortal = 1;
	st_.immortalStartTime = static_cast<float>(SDL_GetTicks64()) / 1000.0f;
	Mix_PlayChannel(-1, sound, 0);
}

void Player::heal(int maxLives) {
	st_.lives++;
	if (st_.lives > maxLives) st_.lives = maxLives;
}

void Player::updateImmortal(float nowSec) {
	if (nowSec - st_.immortalStartTime >= 1.0f) st_.isImmortal = 0;
}

void Player::integrate(Mix_Chunk* jumpSound) {
	st_.x += st_.dx;
	st_.y += st_.dy;
	st_.dy += cfg::kGravity;
	if (!st_.onLedge) {
		if (st_.dy < 0) {
			st_.status = 2;
			if (!Mix_Playing(1)) Mix_PlayChannel(1, jumpSound, 0);
		}
		else if (st_.dy == 0) {
			st_.status = 3;
		}
		else {
			st_.status = 4;
		}
	}
}

void Player::draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const {
	SDL_Texture* t = nullptr;
	int w = cfg::kJumpW, h = cfg::kJumpH;
	switch (st_.status) {
	case 0: t = a.idle(animFrame(12, cfg::kCharacterFrames)); w = cfg::kIdleW; h = cfg::kIdleH; break;
	case 1: t = a.run(animFrame(8, cfg::kCharacterFrames)); w = cfg::kRunW; h = cfg::kRunH; break;
	case 2: t = a.jump(0); break;
	case 3: t = a.jump(1); break;
	case 4: t = a.jump(animFrame(2, cfg::kCharacterFrames) + 2); break;
	default: return;
	}
	const int x = static_cast<int>(scrollX + st_.x), y = static_cast<int>(st_.y);
	const SDL_RendererFlip flip = st_.flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	if (st_.isTakenDamage && st_.isImmortal) {  // blink red, natural size x2 (render.c draw_player_hit)
		SDL_Rect d{ x, y, 0, 0 };
		SDL_QueryTexture(t, nullptr, nullptr, &d.w, &d.h);
		d.w *= 2;
		d.h *= 2;
		const bool red = (SDL_GetTicks64() / 100) % 2 != 0;
		if (red) SDL_SetTextureColorMod(t, 255, 0, 0);
		SDL_RenderCopyEx(r, t, nullptr, &d, 0, nullptr, flip);
		if (red) SDL_SetTextureColorMod(t, 255, 255, 255);
	}
	else {
		const SDL_Rect d{ x, y, w * 2, h * 2 };
		SDL_RenderCopyEx(r, t, nullptr, &d, 0, nullptr, flip);
	}
}
}
```

**`src/game/GigaGolem.h`** (header-only):
```cpp
#pragma once
#include "game/GameAssets.h"
#include "game/Player.h"

namespace jr {
class GigaGolem {
public:
	void reset() { x_ = 0; y_ = (cfg::kWindowH - cfg::kGigaGolemSize) / 2.0f; dx_ = 0; }
	void update(float dt, float scrollX, float playerDx) {  // = updateBoss()
		const float camX = -scrollX;
		if (camX - x_ > 0 && playerDx > 0) dx_ += playerDx;
		else dx_ = static_cast<float>(cfg::kGigaGolemSpeed);
		x_ += dx_ * dt;
	}
	bool touches(const Player& p) const {                   // = touchBoss()
		float px, py, pw, ph;
		p.hitInfo(px, py, pw, ph);
		return px <= (x_ + 0.75 * cfg::kGigaGolemSize);
	}
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) const {
		const SDL_Rect src{ 24 + animFrame(10, cfg::kGigaGolemFrames) * 100, 624, 50, 50 };
		const SDL_Rect d{ static_cast<int>(scrollX + x_), static_cast<int>(y_), cfg::kGigaGolemSize, cfg::kGigaGolemSize };
		SDL_RenderCopyEx(r, a.tex(TextureId::Golem), &src, &d, 0, nullptr, SDL_FLIP_NONE);
	}
	float x() const { return x_; }
private:
	float x_ = 0, y_ = 0, dx_ = 0;
};
}
```

**`src/game/Parallax.h`** (header-only):
```cpp
#pragma once
#include "game/GameAssets.h"

namespace jr {
// = drawBackground(): the scroll state is updated while drawing, so draw() is not const.
class Parallax {
public:
	void reset() {
		for (int i = 0; i < 5; ++i) {
			defaultX_[i] = 0;
			previousX_[i] = -cfg::kWindowW;
			nextX_[i] = cfg::kWindowW;
		}
	}
	void draw(SDL_Renderer* r, const GameAssets& a, float scrollX) {
		constexpr int W = cfg::kWindowW, H = cfg::kWindowH;
		SDL_RenderCopy(r, a.background(0), nullptr, nullptr);
		for (int i = 1; i < 5; ++i) {
			SDL_Texture* t = a.background(i);
			const float dist = scrollX / cfg::kParallaxSpeed[i];
			const SDL_Rect prev{ static_cast<int>(dist + previousX_[i]), 0, W, H };
			const SDL_Rect def{ static_cast<int>(dist + defaultX_[i]), 0, W, H };
			const SDL_Rect next{ static_cast<int>(dist + nextX_[i]), 0, W, H };
			SDL_RenderCopy(r, t, nullptr, &def);
			SDL_RenderCopy(r, t, nullptr, &prev);
			SDL_RenderCopy(r, t, nullptr, &def);
			SDL_RenderCopy(r, t, nullptr, &def);
			SDL_RenderCopy(r, t, nullptr, &next);
			if (-dist > defaultX_[i] + W) {
				defaultX_[i] += W;
				previousX_[i] = defaultX_[i] - W;
				nextX_[i] = defaultX_[i] + W;
			}
			else if (-dist < defaultX_[i] - W) {
				defaultX_[i] -= W;
				previousX_[i] = defaultX_[i] - W;
				nextX_[i] = defaultX_[i] + W;
			}
		}
	}
	static void drawStatic(SDL_Renderer* r, const GameAssets& a) {  // ending screen (render_over)
		for (int i = 0; i < 5; ++i) SDL_RenderCopy(r, a.background(i), nullptr, nullptr);
	}
private:
	float defaultX_[5]{}, previousX_[5]{}, nextX_[5]{};
};
}
```

**`src/game/Hud.h` + `.cpp`:**
```cpp
// Hud.h
#pragma once
#include "core/Sdl.h"

namespace jr {
class GameAssets;

class Hud {
public:
	void drawStatus(SDL_Renderer* r, const GameAssets& a, int lives, int potions);       // = render_status()
	void drawOver(SDL_Renderer* r, const GameAssets& a, const char* title, SDL_Color fg, bool showHint);
	void resetOver() { overTitle_.reset(); overHint_.reset(); }
private:
	void label(SDL_Renderer* r, TTF_Font* font, sdl::TexturePtr& tex, int& cached, int value);
	sdl::TexturePtr lives_, potions_, overTitle_, overHint_;
	int cachedLives_ = 0, cachedPotions_ = 0;
};
}
```
```cpp
// Hud.cpp
#include "game/Hud.h"
#include "game/GameAssets.h"

namespace jr {
void Hud::label(SDL_Renderer* r, TTF_Font* font, sdl::TexturePtr& tex, int& cached, int value) {
	if (tex && cached == value) return;
	tex = sdl::renderText(r, font, std::to_string(value), SDL_Color{ 255, 255, 255, 255 });
	cached = value;
}

void Hud::drawStatus(SDL_Renderer* r, const GameAssets& a, int lives, int potions) {
	label(r, a.hudFont(), lives_, cachedLives_, lives);
	label(r, a.hudFont(), potions_, cachedPotions_, potions);
	const SDL_Rect t1 = sdl::sized(lives_.get(), 5, -15);
	const SDL_Rect t2 = sdl::sized(potions_.get(), 200, -15);
	const SDL_Rect src1{ animFrame(6, cfg::kItemFrames) * 32, 1 * 32, 32, 32 };
	const SDL_Rect src2{ animFrame(8, cfg::kItemFrames) * 32, 9 * 32, 32, 32 };
	const SDL_Rect icon1{ t1.x + t1.w, 0, 48, 48 }, icon2{ t2.x + t2.w, 0, 48, 48 };
	SDL_RenderCopy(r, lives_.get(), nullptr, &t1);
	SDL_RenderCopy(r, a.tex(TextureId::Items), &src1, &icon1);
	SDL_RenderCopy(r, potions_.get(), nullptr, &t2);
	SDL_RenderCopy(r, a.tex(TextureId::Items), &src2, &icon2);
}

void Hud::drawOver(SDL_Renderer* r, const GameAssets& a, const char* title, SDL_Color fg, bool showHint) {
	if (!overTitle_) overTitle_ = sdl::renderText(r, a.overFont(), title, fg);
	SDL_Rect t = sdl::sized(overTitle_.get(), 0, 0);
	t.x = (cfg::kWindowW - t.w) / 2;
	t.y = (cfg::kWindowH - t.h) / 2;
	SDL_RenderCopy(r, overTitle_.get(), nullptr, &t);
	if (!showHint) return;
	if (!overHint_) overHint_ = sdl::renderText(r, a.hudFont(), "Press R to retry, X to return menu", SDL_Color{ 255, 255, 73, 255 });
	const SDL_Rect h = sdl::sized(overHint_.get(), 0, -15);
	SDL_RenderCopy(r, overHint_.get(), nullptr, &h);
}
}
```

### Task 1.8: Level + World

**`src/game/Level.h` + `.cpp`:**
```cpp
// Level.h
#pragma once
#include "game/Entity.h"
#include "game/LevelLoader.h"
#include <SDL_mixer.h>
#include <memory>
#include <vector>

namespace jr {
class GameAssets;
class Player;
class World;

struct Segment {
	std::vector<std::unique_ptr<Entity>> entities;  // row-major, non-empty cells only
};

class Level {
public:
	void build(const Course& c);                                    // from in-memory grids (also on Retry)
	void swapOnKey(const Course& c);                                // rebuild the onkey segment in place
	void collideGround(Player& p, Mix_Chunk* landing) const;        // = collision_detect_floor
	void collideSegments(Player& p, World& w, float dt);            // = collision_detect_map, course order
	void animate(Uint64 nowMs);
	void drawSegments(SDL_Renderer* r, const GameAssets& a, float scrollX) const;
	void drawGrounds(SDL_Renderer* r, const GameAssets& a, float scrollX) const;  // = drawLedges
	template <class F>
	void forEachEntity(F&& f) const {
		for (const auto& s : segments_)
			for (const auto& e : s.entities) f(*e);
	}
private:
	static Segment makeSegment(const Grid& g, float x);
	std::vector<SDL_FRect> grounds_;
	std::vector<Segment> segments_;
};
}
```
```cpp
// Level.cpp
#include "game/Level.h"
#include "game/EntityFactory.h"
#include "game/GameAssets.h"
#include "game/Player.h"
#include "game/World.h"

namespace jr {
Segment Level::makeSegment(const Grid& g, float x) {
	Segment seg;
	for (int row = 0; row < cfg::kGridRows; ++row) {
		for (int col = 0; col < cfg::kGridCols; ++col) {
			if (auto e = makeEntity(g[row][col], x + col * cfg::kTile, static_cast<float>(row * cfg::kTile))) {
				seg.entities.push_back(std::move(e));
			}
		}
	}
	return seg;
}

void Level::build(const Course& c) {
	grounds_.clear();
	segments_.clear();
	for (float x : c.groundX) {
		grounds_.push_back({ x, static_cast<float>(cfg::kWindowH - cfg::kGroundH), static_cast<float>(cfg::kGroundW), static_cast<float>(cfg::kGroundH) });
	}
	for (const auto& s : c.segments) segments_.push_back(makeSegment(c.grids[s.grid], s.x));
}

void Level::swapOnKey(const Course& c) {
	if (c.onKeySegment < 0) return;
	segments_[c.onKeySegment] = makeSegment(c.grids[c.onKeyGrid], c.segments[c.onKeySegment].x);
}

void Level::collideGround(Player& p, Mix_Chunk* landing) const {
	for (const SDL_FRect& g : grounds_) p.resolveAgainst(g, landing);  // no intersection gate, no clearHurt (as C)
}

void Level::collideSegments(Player& p, World& w, float dt) {
	for (auto& seg : segments_) {
		for (auto& e : seg.entities) {
			const SDL_FRect b = e->bounds();  // before the golem moves (C reads bx/by before updateGolem)
			const SDL_Rect pr = p.hitRect();
			const SDL_Rect er = toIntRect(b);
			e->preCollide(p, dt);
			if (e->grantsKey()) w.setKeyObtained();  // checked before the hit test: takes effect next frame
			if (!SDL_HasIntersection(&pr, &er)) continue;
			e->onTouch(p, w);
			if (!e->isSolid()) {
				if (e->isLethal() && !p.state().isImmortal) p.hurt(w.assets().hurt(e->hurtSound()));
			}
			else {
				p.clearHurt();
				p.resolveAgainst(b, w.assets().landing());
			}
		}
	}
}

void Level::animate(Uint64 nowMs) {
	for (auto& seg : segments_)
		for (auto& e : seg.entities) e->animate(nowMs);
}

void Level::drawSegments(SDL_Renderer* r, const GameAssets& a, float scrollX) const {
	forEachEntity([&](const Entity& e) { e.draw(r, a, scrollX); });
}

void Level::drawGrounds(SDL_Renderer* r, const GameAssets& a, float scrollX) const {
	for (const SDL_FRect& g : grounds_) {
		const SDL_Rect src{ 217, 116, static_cast<int>(g.w), static_cast<int>(g.h) };
		const SDL_Rect d{ static_cast<int>(scrollX + g.x), static_cast<int>(g.y), static_cast<int>(g.w), static_cast<int>(g.h) };
		SDL_RenderCopyEx(r, a.tex(TextureId::Ground), &src, &d, 0, nullptr, SDL_FLIP_NONE);
	}
}
}
```
Kiểm tra chéo với C: trong `collision_detect_map`, `isTakenDamage = 0` + correction nằm ở nhánh `isBlocked`; nhánh `!isBlocked` chỉ trừ máu khi `isLethal && !isImmortal` — khớp. Item lấy trước (onTouch) rồi mới xét blocked — khớp.

**`src/game/World.h` + `.cpp`:**
```cpp
// World.h
#pragma once
#include "game/GigaGolem.h"
#include "game/Hud.h"
#include "game/Level.h"
#include "game/Parallax.h"
#include "game/Player.h"

namespace jr {
enum class Status { GameOver = 0, TrueEnd = 1, GoodEnd = 2, Playing = 3 };  // 0-2 = ending music index

class World {
public:
	World(const GameAssets& a, const Course& c, int difficulty) : assets_(a), course_(c), difficulty_(difficulty) {}
	World(const World&) = delete;
	World& operator=(const World&) = delete;

	void reset();                                    // = game_reset (new run, Retry)
	void input(const InputState& in) { player_.input(in); }
	void jump() { player_.jump(); }
	void step(float dt);                             // floor collision, segment collision, physics
	void render(SDL_Renderer* r);                    // = do_render (clear ... present)
	bool overReady() const;                          // ending screen shown >= 2 s

	// used by entities
	const GameAssets& assets() const { return assets_; }
	int difficulty() const { return difficulty_; }
	void addPotion() { ++potions_; }
	void setKeyObtained() { keyObtained_ = true; }

	// read-only state (HUD, tools/parity)
	const Player& player() const { return player_; }
	Player& player() { return player_; }
	const Level& level() const { return level_; }
	int potions() const { return potions_; }
	bool keyObtained() const { return keyObtained_; }
	Status status() const { return status_; }
	bool isOver() const { return isOver_; }
	float bossX() const { return boss_.x(); }
	float scrollX() const { return scrollX_; }

private:
	void physics(float dt);                          // = add_physics
	void renderOver(SDL_Renderer* r, const char* title, SDL_Color fg);

	const GameAssets& assets_;
	const Course& course_;
	int difficulty_;
	Player player_;
	GigaGolem boss_;
	Level level_;
	Parallax parallax_;
	Hud hud_;
	Status status_ = Status::Playing;
	bool isOver_ = false, keyObtained_ = false, swapped_ = false;
	int potions_ = 0;
	float overTiming_ = 0, bossStart_ = 0, scrollX_ = 0;
};
}
```
```cpp
// World.cpp
#include "game/World.h"

namespace jr {
namespace {
float nowSec() { return static_cast<float>(SDL_GetTicks64()) / 1000.0f; }
}

void World::reset() {
	status_ = Status::Playing;
	isOver_ = false;
	overTiming_ = 0;
	player_.reset(difficulty_ == cfg::kDifficultyMedium ? cfg::kDifficultyMedium
		: difficulty_ == cfg::kDifficultyHard ? cfg::kDifficultyHard : cfg::kDifficultyEasy);
	potions_ = 0;
	keyObtained_ = false;
	swapped_ = false;
	bossStart_ = nowSec();
	boss_.reset();
	scrollX_ = 0;
	parallax_.reset();
	hud_.resetOver();
	level_.build(course_);
}

void World::step(float dt) {
	level_.collideGround(player_, assets_.landing());
	level_.collideSegments(player_, *this, dt);
	physics(dt);
}

void World::physics(float dt) {
	const PlayerState& s = player_.state();
	if (s.x >= course_.finish && !isOver_) {
		isOver_ = true;
		Mix_HaltMusic();
		overTiming_ = nowSec();
		if (potions_ < cfg::kRequiredPotions) status_ = Status::TrueEnd;
		else if (potions_ == cfg::kRequiredPotions) status_ = Status::GoodEnd;
	}
	else if ((boss_.touches(player_) || s.lives == 0 || s.y > cfg::kWindowH + 250) && !isOver_) {
		isOver_ = true;
		Mix_HaltChannel(-1);
		Mix_HaltChannel(1);
		Mix_HaltMusic();
		overTiming_ = nowSec();
		status_ = Status::GameOver;
	}
	else {
		if (s.isImmortal) player_.updateImmortal(nowSec());
		player_.integrate(assets_.jumpSound());
		scrollX_ = -player_.state().x + 350;
		if (scrollX_ > 0) scrollX_ = 0;
		if (nowSec() - bossStart_ >= 0.5f) boss_.update(dt, scrollX_, player_.state().dx);
	}
}

bool World::overReady() const { return isOver_ && nowSec() - overTiming_ >= 2.0f; }

void World::renderOver(SDL_Renderer* r, const char* title, SDL_Color fg) {
	Mix_HaltChannel(-1);
	if (!Mix_PlayingMusic()) Mix_PlayMusic(assets_.endingMusic(static_cast<int>(status_)), -1);
	Parallax::drawStatic(r, assets_);
	hud_.drawOver(r, assets_, title, fg, nowSec() - overTiming_ >= 2.0f);
}

void World::render(SDL_Renderer* r) {
	SDL_RenderClear(r);
	switch (status_) {
	case Status::GameOver: renderOver(r, "Bad End", SDL_Color{ 210, 8, 45, 255 }); break;
	case Status::TrueEnd: renderOver(r, "True End", SDL_Color{ 200, 112, 126, 255 }); break;
	case Status::GoodEnd: renderOver(r, "Good End", SDL_Color{ 96, 106, 116, 255 }); break;
	case Status::Playing:
		parallax_.draw(r, assets_, scrollX_);
		if (keyObtained_ && !swapped_) {  // = draw_key_map in do_render, before the maps are drawn
			level_.swapOnKey(course_);
			swapped_ = true;
		}
		level_.animate(SDL_GetTicks64());
		level_.drawSegments(r, assets_, scrollX_);
		level_.drawGrounds(r, assets_, scrollX_);
		player_.draw(r, assets_, scrollX_);
		boss_.draw(r, assets_, scrollX_);
		hud_.drawStatus(r, assets_, player_.state().lives, potions_);
		if (!Mix_PlayingMusic()) Mix_PlayMusic(assets_.inGameMusic(), -1);
		break;
	}
	SDL_RenderPresent(r);
}
}
```

### Task 1.9: Thêm C++ vào project, build

**Files:** Modify `Jungle Run.vcxproj` (+ `.filters`)

**Step 1:** Thêm vào ItemGroup ClCompile, mỗi file kèm `/W4` riêng (file `.c` giữ Level3):
```xml
    <ClCompile Include="src\core\Sdl.cpp"><WarningLevel>Level4</WarningLevel></ClCompile>
    <ClCompile Include="src\game\GameAssets.cpp"><WarningLevel>Level4</WarningLevel></ClCompile>
    <ClCompile Include="src\game\Pickup.cpp"><WarningLevel>Level4</WarningLevel></ClCompile>
    <ClCompile Include="src\game\SmolGolem.cpp"><WarningLevel>Level4</WarningLevel></ClCompile>
    <ClCompile Include="src\game\EntityFactory.cpp"><WarningLevel>Level4</WarningLevel></ClCompile>
    <ClCompile Include="src\game\LevelLoader.cpp"><WarningLevel>Level4</WarningLevel></ClCompile>
    <ClCompile Include="src\game\Player.cpp"><WarningLevel>Level4</WarningLevel></ClCompile>
    <ClCompile Include="src\game\Hud.cpp"><WarningLevel>Level4</WarningLevel></ClCompile>
    <ClCompile Include="src\game\Level.cpp"><WarningLevel>Level4</WarningLevel></ClCompile>
    <ClCompile Include="src\game\World.cpp"><WarningLevel>Level4</WarningLevel></ClCompile>
```
ClInclude: `src\core\Sdl.h`, `src\game\Config.h`, `GameAssets.h`, `Entity.h`, `Tile.h`, `Trap.h`, `Pickup.h`, `SmolGolem.h`, `Decoration.h`, `EntityFactory.h`, `LevelLoader.h`, `Player.h`, `GigaGolem.h`, `Parallax.h`, `Hud.h`, `Level.h`, `World.h`. Filters: thêm các file vào `Source Files\core` / `Source Files\game`.

**Step 2:** Build Debug + Release. Expected: 0 lỗi; `grep -E "\.(cpp|h)\([0-9,]+\): warning" /tmp/jr-Debug.log` không có output. Không trùng `.obj` (kiểm: `ls x64/Debug/"Jungle Run"/*.obj | wc -l` = số file `.c` + 10).

**Step 3: Commit**
```bash
git add src/core/Sdl.* src/game/*.h src/game/*.cpp "Jungle Run.vcxproj" "Jungle Run.vcxproj.filters"
git commit -m "feat(game): C++ gameplay model (entities, level loader, world) built alongside C"
```

### Task 1.10: Kiểm tra song song `tools/parity`

**Files:** Create `tools/parity/ParityCheck.vcxproj`, `parity_clock.h`, `parity_api.h`, `parity_c.c`, `parity_cpp.cpp`, `parity_main.cpp`

**`parity_clock.h`** (forced-include cho mọi file game C và C++, **không** cho `parity_main.cpp`):
```c
#pragma once
#include <SDL.h>
#ifdef __cplusplus
extern "C"
#endif
Uint64 parity_ticks(void);
#define SDL_GetTicks64 parity_ticks
```

**`parity_api.h`:**
```c
#pragma once
#include <SDL.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { int left, right, down, jump; } ParityInput;

#define PARITY_MAX_CELLS 8192
typedef struct {
	float x, y, dx, dy, immortalStartTime, bossX, scrollX;
	int lives, isTakenDamage, isImmortal, onLedge, flip, status;
	int potions, keyObtained, statusState, isOver;
	int cells;
	unsigned char lethal[PARITY_MAX_CELLS], solid[PARITY_MAX_CELLS], taken[PARITY_MAX_CELLS];
	float cellX[PARITY_MAX_CELLS];
} ParitySnap;

int c_init(SDL_Renderer* r, int difficulty);
void c_place(float x, float y);
void c_frame(ParityInput in, float dt);
void c_snap(ParitySnap* s);
int c_segment_count(void);
float c_segment_x(int i);
int c_find_key(float* x, float* y);

int cpp_init(SDL_Renderer* r, int difficulty);
void cpp_place(float x, float y);
void cpp_frame(ParityInput in, float dt);
void cpp_snap(ParitySnap* s);
void cpp_shutdown(void);
#ifdef __cplusplus
}
#endif
```

**`parity_c.c`** (phía C — dùng nguyên `src/game/*.c`, không nạp asset):
```c
#include "game/game.h"
#include "parity_api.h"
#include <math.h>
#include <string.h>

static GameState gs;  /* C side only; destroyed with the process */

int c_init(SDL_Renderer* r, int difficulty) {
	memset(&gs, 0, sizeof gs);
	gs.renderer = r;
	gs.difficulty = difficulty;
	game_reset(&gs);
	return 1;
}

void c_place(float x, float y) { gs.player.x = x; gs.player.y = y; }

/* Literal copy of the movement part of handle_input() in src/scenes/gameplay.c, keys from `in`. */
static void c_input(ParityInput in) {
	if (in.left) {
		gs.player.status = 1;
		gs.player.flip = 1;
		gs.player.dx -= 0.3f;
		if (gs.player.dx < -2.5f) gs.player.dx = -2.5f;
	}
	else if (in.right) {
		gs.player.status = 1;
		gs.player.flip = 0;
		gs.player.dx += 0.3f;
		if (gs.player.dx > 2.5f) gs.player.dx = 2.5f;
	}
	else if (in.down) {
		gs.player.dy += 0.3f;
	}
	else {
		gs.player.status = 0;
		gs.player.flip = 0;
		gs.player.dx *= 0.8f;
		if (fabsf(gs.player.dx) < 0.1f) gs.player.dx = 0;
	}
	if (in.jump && gs.player.onLedge) {
		gs.player.onLedge = 0;
		gs.player.dy = -9;
	}
}

void c_frame(ParityInput in, float dt) {  /* same order as gameplay_run() */
	gs.dt = dt;
	c_input(in);
	collision_detect_floor(&gs);
	for (int i = 0; i < gs.map->counter; i++) collision_detect_map(&gs, &gs.map[i]);
	add_physics(&gs);
	do_render(&gs);
}

void c_snap(ParitySnap* s) {
	s->x = gs.player.x; s->y = gs.player.y; s->dx = gs.player.dx; s->dy = gs.player.dy;
	s->immortalStartTime = gs.player.immortalStartTime;
	s->bossX = gs.gigaGolem.pos.x; s->scrollX = gs.scrollX;
	s->lives = gs.player.lives; s->isTakenDamage = gs.player.isTakenDamage; s->isImmortal = gs.player.isImmortal;
	s->onLedge = gs.player.onLedge; s->flip = gs.player.flip; s->status = gs.player.status;
	s->potions = gs.health_potion_counter; s->keyObtained = gs.isKeyObtained;
	s->statusState = gs.statusState; s->isOver = gs.isOver;
	s->cells = 0;
	for (int m = 0; m < gs.map->counter; m++)
		for (int row = 0; row < MAP_HEIGHT; row++)
			for (int col = 0; col < MAP_WIDTH; col++) {
				if (gs.map[m].pos[row][col] == 0 || s->cells == PARITY_MAX_CELLS) continue;
				const Ledge* l = &gs.map[m].ledges[row][col];
				s->lethal[s->cells] = (unsigned char)l->isLethal;
				s->solid[s->cells] = (unsigned char)l->isBlocked;
				s->taken[s->cells] = (unsigned char)l->isObtained;
				s->cellX[s->cells] = l->x;
				s->cells++;
			}
}

int c_segment_count(void) { return gs.map->counter; }
float c_segment_x(int i) { return gs.map[i].ledges[0][0].x; }

int c_find_key(float* x, float* y) {
	for (int m = 0; m < gs.map->counter; m++)
		for (int row = 0; row < MAP_HEIGHT; row++)
			for (int col = 0; col < MAP_WIDTH; col++)
				if (gs.map[m].pos[row][col] == 21) {
					*x = gs.map[m].ledges[row][col].x;
					*y = gs.map[m].ledges[row][col].y;
					return 1;
				}
	return 0;
}
```

**`parity_cpp.cpp`** (phía C++ — `World` với `GameAssets` chưa nạp: mọi texture/âm thanh là null, vô hại):
```cpp
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
	s->x = p.x; s->y = p.y; s->dx = p.dx; s->dy = p.dy; s->immortalStartTime = p.immortalStartTime;
	s->bossX = world->bossX(); s->scrollX = world->scrollX();
	s->lives = p.lives; s->isTakenDamage = p.isTakenDamage; s->isImmortal = p.isImmortal;
	s->onLedge = p.onLedge; s->flip = p.flip; s->status = p.status;
	s->potions = world->potions(); s->keyObtained = world->keyObtained() ? 1 : 0;
	s->statusState = static_cast<int>(world->status()); s->isOver = world->isOver() ? 1 : 0;
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

extern "C" void cpp_shutdown(void) { world.reset(); assets.reset(); course.reset(); }
```

**`parity_main.cpp`:**
```cpp
#include "parity_api.h"
#include <SDL_mixer.h>
#include <cstdio>
#include <cstring>
#include <memory>

namespace {
Uint64 ticks = 100000;
constexpr int kFrames = 3000;
constexpr float kDt = 0.016f;

ParityInput pattern(int kind, int f) {
	ParityInput in{};
	switch (kind) {
	case 0: break;                                                                       // idle
	case 1: in.right = 1; break;                                                         // run right
	case 2: in.right = 1; in.jump = f % 40 == 0; break;                                  // run + jump
	case 3: in.right = f % 90 < 60; in.left = f % 90 >= 60; in.jump = f % 25 == 0; break; // zig-zag + jump
	case 4: in.right = 1; in.down = f % 3 == 0; break;                                   // run + down
	}
	return in;
}

bool compare(const ParitySnap& c, const ParitySnap& p, const char* scenario, int frame) {
	auto fail = [&](const char* field, double cv, double pv) {
		std::printf("MISMATCH %s frame %d %s: C=%.9g C++=%.9g\n", scenario, frame, field, cv, pv);
		return false;
	};
#define F(name) if (c.name != p.name) return fail(#name, c.name, p.name)
	F(x); F(y); F(dx); F(dy); F(immortalStartTime); F(bossX); F(scrollX);
	F(lives); F(isTakenDamage); F(isImmortal); F(onLedge); F(flip); F(status);
	F(potions); F(keyObtained); F(statusState); F(isOver); F(cells);
#undef F
	for (int i = 0; i < c.cells; ++i) {
		if (c.lethal[i] != p.lethal[i] || c.solid[i] != p.solid[i] || c.taken[i] != p.taken[i] || c.cellX[i] != p.cellX[i]) {
			std::printf("MISMATCH %s frame %d cell %d (x %.1f): lethal %d/%d solid %d/%d taken %d/%d x %.9g/%.9g\n",
				scenario, frame, i, c.cellX[i], c.lethal[i], p.lethal[i], c.solid[i], p.solid[i], c.taken[i], p.taken[i], c.cellX[i], p.cellX[i]);
			return false;
		}
	}
	return true;
}

bool runScenario(SDL_Renderer* r, const char* name, int difficulty, int kind, bool place, float x, float y, int idleFirst) {
	static ParitySnap cs, ps;  // ~40KB each: off the stack
	ticks += 1000;
	if (!c_init(r, difficulty) || !cpp_init(r, difficulty)) return false;
	if (place) { c_place(x, y); cpp_place(x, y); }
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
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) { std::printf("SDL_Init: %s\n", SDL_GetError()); return 2; }
	Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048);
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_CRITICAL);  // null fonts/textures log every frame
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
```

**`ParityCheck.vcxproj`** (chỉ Debug|x64, Console). Điểm chính:
```xml
<PropertyGroup Label="Globals"> ... <ProjectGuid>{7d3c2a51-9e0b-4c8a-b1f2-3e4d5c6b7a81}</ProjectGuid> ... </PropertyGroup>
<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
  <IncludePath>$(ProjectDir)..\..\External library\SDL2\include;$(ProjectDir)..\..\External library\SDL2_ttf\include;$(ProjectDir)..\..\External library\SDL2_image\include;$(ProjectDir)..\..\External library\SDL2_mixer\include;$(IncludePath)</IncludePath>
  <LibraryPath>$(ProjectDir)..\..\External library\SDL2\lib\x64;$(ProjectDir)..\..\External library\SDL2_image\lib\x64;$(ProjectDir)..\..\External library\SDL2_ttf\lib\x64;$(ProjectDir)..\..\External library\SDL2_mixer\lib\x64;$(LibraryPath)</LibraryPath>
</PropertyGroup>
<ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
  <ClCompile>
    <PreprocessorDefinitions>_DEBUG;_CONSOLE;_CRT_SECURE_NO_WARNINGS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
    <AdditionalIncludeDirectories>$(ProjectDir)..\..\src;$(ProjectDir);%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
    <LanguageStandard>stdcpp17</LanguageStandard>
    <ConformanceMode>true</ConformanceMode>
    <ForcedIncludeFiles>parity_clock.h</ForcedIncludeFiles>
  </ClCompile>
  <Link>
    <SubSystem>Console</SubSystem>
    <AdditionalDependencies>SDL2.lib;SDL2main.lib;SDL2_image.lib;SDL2_ttf.lib;SDL2_mixer.lib;%(AdditionalDependencies)</AdditionalDependencies>
  </Link>
</ItemDefinitionGroup>
<ItemGroup>
  <!-- C reference -->
  <ClCompile Include="..\..\src\core\res.c" />
  <ClCompile Include="..\..\src\game\assets.c" />
  <ClCompile Include="..\..\src\game\physics.c" />
  <ClCompile Include="..\..\src\game\collision.c" />
  <ClCompile Include="..\..\src\game\map.c" />
  <ClCompile Include="..\..\src\game\levels.c" />
  <ClCompile Include="..\..\src\game\render.c" />
  <ClCompile Include="parity_c.c" />
  <!-- C++ port -->
  <ClCompile Include="..\..\src\core\Sdl.cpp" />
  <ClCompile Include="..\..\src\game\GameAssets.cpp" />
  <ClCompile Include="..\..\src\game\Pickup.cpp" />
  <ClCompile Include="..\..\src\game\SmolGolem.cpp" />
  <ClCompile Include="..\..\src\game\EntityFactory.cpp" />
  <ClCompile Include="..\..\src\game\LevelLoader.cpp" />
  <ClCompile Include="..\..\src\game\Player.cpp" />
  <ClCompile Include="..\..\src\game\Hud.cpp" />
  <ClCompile Include="..\..\src\game\Level.cpp" />
  <ClCompile Include="..\..\src\game\World.cpp" />
  <ClCompile Include="parity_cpp.cpp" />
  <ClCompile Include="parity_main.cpp"><ForcedIncludeFiles></ForcedIncludeFiles></ClCompile>
</ItemGroup>
```
(Các phần còn lại của vcxproj — ProjectConfigurations, imports `Microsoft.Cpp.*.props/targets`, `ConfigurationType=Application`, `PlatformToolset=v143` — chép từ `Jungle Run.vcxproj`.)

**Step: Build + chạy** (từ thư mục gốc repo để thấy `Resource\Levels` và DLL):
```bash
"$MSB" tools/parity/ParityCheck.vcxproj -p:Configuration=Debug -p:Platform=x64 -p:PlatformToolset=v145 -nologo -v:minimal
./tools/parity/x64/Debug/ParityCheck.exe
```
Expected: `PARITY OK: 82 scenarios x 3000 frames`, exit 0.

### Task 1.11: Sửa lệch cho tới khi khớp, commit GĐ1

**Step 1:** Mỗi `MISMATCH` chỉ ra frame + trường đầu tiên lệch. Sửa **phía C++** cho khớp C (C là chuẩn). Không sửa code C, không nới so sánh. Nếu nghi lỗi nằm ở harness (vd. thứ tự gọi trong `c_frame`), đối chiếu với `src/scenes/gameplay.c` trước khi sửa.

**Step 2:** Build lại `Jungle Run.vcxproj` (Debug + Release) — 0 lỗi, `.cpp` 0 warning.

**Step 3: Commit**
```bash
git add tools/parity src "Jungle Run.vcxproj" "Jungle Run.vcxproj.filters"
git commit -m "test(parity): frame-by-frame C vs C++ gameplay check; PARITY OK"
```

---

## GĐ2 — Khung ứng dụng C++ và xóa C

### Task 2.1: `src/core/Game.h` + `.cpp`

```cpp
// Game.h
#pragma once
#include "core/Sdl.h"
#include "game/Config.h"
#include <memory>
#include <string>

namespace jr {
enum class SceneId { Quit, Menu, Story, Help, Donate, GameMode, Intro, Gameplay };
class Scene;

struct Settings {
	int difficulty = cfg::kDifficultyEasy;
	int modeIndex = 0;  // 0 Easy, 1 Medium, 2 Hard (Game Mode label)
};

// Owns the SDL session, the single window/renderer, the audio device and menu sounds.
// Member order is destruction order in reverse: sounds -> audio -> renderer -> window -> SDL_Quit.
class Game {
public:
	Game();
	Game(const Game&) = delete;
	Game& operator=(const Game&) = delete;
	bool ok() const { return ok_; }
	int run();
	SDL_Renderer* renderer() const { return renderer_.get(); }
	Mix_Chunk* click() const { return click_.get(); }
	Mix_Music* menuMusic() const { return menuMusic_.get(); }
	Settings& settings() { return settings_; }
	void resize(int w, int h);
	void tick();                         // Debug: live counts in the window title once per second
	void fatal(const std::string& msg);  // log + message box
private:
	std::unique_ptr<Scene> makeScene(SceneId id);
	sdl::Session session_;
	sdl::WindowPtr window_;
	sdl::RendererPtr renderer_;
	sdl::AudioDevice audio_;
	sdl::ChunkPtr click_;
	sdl::MusicPtr menuMusic_;
	Settings settings_;
	bool ok_ = false;
	Uint32 lastTitle_ = 0;
};
}
```
```cpp
// Game.cpp
#include "core/Game.h"
#include "scenes/Scenes.h"

namespace jr {
namespace {
#ifdef _DEBUG
const char* name(SceneId id) {
	static const char* const names[] = { "QUIT", "MENU", "STORY", "HELP", "DONATE", "GAMEMODE", "INTRO", "GAMEPLAY" };
	return names[static_cast<int>(id)];
}
#endif
}

Game::Game() {
	if (!session_.ok()) {
		fatal(SDL_GetError());
		return;
	}
	window_.reset(SDL_CreateWindow("Jungle Run", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, cfg::kMenuW, cfg::kMenuH, 0));
	if (!window_) {
		fatal(SDL_GetError());
		return;
	}
	renderer_.reset(SDL_CreateRenderer(window_.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
	if (!renderer_) {
		fatal(SDL_GetError());
		return;
	}
	click_ = sdl::loadChunk("Resource\\Sound\\Sound Effects\\click.wav");
	menuMusic_ = sdl::loadMusic("Resource\\Sound\\Soundtrack\\MainMenu2.mp3");
	ok_ = true;
}

std::unique_ptr<Scene> Game::makeScene(SceneId id) {
	switch (id) {
	case SceneId::Menu: return std::make_unique<MenuScene>(*this);
	case SceneId::Story: return std::make_unique<StoryScene>(*this);
	case SceneId::Help: return std::make_unique<HelpScene>(*this);
	case SceneId::Donate: return std::make_unique<DonateScene>(*this);
	case SceneId::GameMode: return std::make_unique<GameModeScene>(*this);
	case SceneId::Intro: return std::make_unique<IntroScene>(*this);
	case SceneId::Gameplay: return std::make_unique<GameplayScene>(*this);
	default: return nullptr;
	}
}

int Game::run() {
	SceneId id = SceneId::Menu;
	while (id != SceneId::Quit) {
		const SceneId prev = id;
		{
			auto scene = makeScene(id);
			id = scene ? scene->run() : SceneId::Quit;
		}  // scene and everything it loaded is released here
#ifdef _DEBUG
		SDL_Log("[scene %s -> %s] %s", name(prev), name(id), sdl::liveCounts().c_str());
#else
		(void)prev;
#endif
	}
	return 0;
}

void Game::resize(int w, int h) {
	int cw = 0, ch = 0;
	SDL_GetWindowSize(window_.get(), &cw, &ch);
	if (cw == w && ch == h) return;
	SDL_SetWindowSize(window_.get(), w, h);
	SDL_SetWindowPosition(window_.get(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void Game::tick() {
#ifdef _DEBUG
	const Uint32 now = SDL_GetTicks();
	if (now - lastTitle_ < 1000) return;
	lastTitle_ = now;
	SDL_SetWindowTitle(window_.get(), ("Jungle Run [" + sdl::liveCounts() + "]").c_str());
#endif
}

void Game::fatal(const std::string& msg) {
	SDL_Log("fatal: %s", msg.c_str());
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Jungle Run", msg.c_str(), window_.get());
}
}
```

### Task 2.2: `src/scenes/Scene.h` + `.cpp`, `src/ui/Button.h` + `.cpp`

```cpp
// Scene.h
#pragma once
#include "core/Game.h"

namespace jr {
// Template Method loop for the menu scenes. Intro and Gameplay override run() to keep the C loop order.
// Resources are members: they are released when Game destroys the scene.
class Scene {
public:
	Scene(Game& game, SceneId self) : game_(game), self_(self), next_(self) {}
	virtual ~Scene() = default;
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
	virtual SceneId run();
protected:
	virtual void handleEvent(const SDL_Event&) {}
	virtual void update(float) {}
	virtual void render(SDL_Renderer*) {}
	void go(SceneId next) { next_ = next; }  // stop polling; leftover events stay queued for the next scene
	bool running() const { return next_ == self_; }
	Game& game_;
	const SceneId self_;
	SceneId next_;
};
}
```
```cpp
// Scene.cpp
#include "scenes/Scene.h"

namespace jr {
SceneId Scene::run() {
	SDL_Renderer* r = game_.renderer();
	Uint64 last = SDL_GetTicks64();
	while (running()) {
		SDL_Event e;
		while (running() && SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) go(SceneId::Quit);
			else handleEvent(e);
		}
		if (!running()) break;
		const Uint64 now = SDL_GetTicks64();
		update(static_cast<float>(now - last) / 1000.0f);
		last = now;
		SDL_RenderClear(r);
		render(r);
		SDL_RenderPresent(r);
		game_.tick();
	}
	return next_;
}
}
```
```cpp
// Button.h
#pragma once
#include "core/Sdl.h"

namespace jr {
// Text that turns `hot` color under the mouse. Hit test is inclusive, like ui_hit() in the C version.
class Button {
public:
	Button(SDL_Renderer* r, TTF_Font* font, const char* text, SDL_Point at, SDL_Color normal, SDL_Color hot);
	bool ok() const { return normal_ != nullptr; }
	void hover(int x, int y) { hot_ = hit(x, y); }
	bool hit(int x, int y) const { return x >= rect_.x && x <= rect_.x + rect_.w && y >= rect_.y && y <= rect_.y + rect_.h; }
	void draw(SDL_Renderer* r) const { SDL_RenderCopy(r, hot_ ? hotTex_.get() : normal_.get(), nullptr, &rect_); }
private:
	sdl::TexturePtr normal_, hotTex_;
	SDL_Rect rect_{};
	bool hot_ = false;
};
}
```
```cpp
// Button.cpp
#include "ui/Button.h"

namespace jr {
Button::Button(SDL_Renderer* r, TTF_Font* font, const char* text, SDL_Point at, SDL_Color normal, SDL_Color hot)
	: normal_(sdl::renderText(r, font, text, normal)), hotTex_(sdl::renderText(r, font, text, hot)),
	  rect_(sdl::sized(normal_.get(), at.x, at.y)) {}
}
```

### Task 2.3: `src/scenes/Scenes.h` + 5 màn menu

**`Scenes.h`:**
```cpp
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
```
(`std::unique_ptr<Button>` vì `Button` dựng sau khi font đã mở trong constructor.)

**`MenuScene.cpp`** — port `src/scenes/menu.c`:
```cpp
#include "scenes/Scenes.h"

namespace jr {
namespace {
const char* const kLabels[6] = { "Single Player", "Story", "Game Mode", "Help", "Donate", "Exit" };
const SDL_Point kAt[6] = { { 400, 170 }, { 440, 225 }, { 410, 280 }, { 450, 335 }, { 430, 390 }, { 450, 445 } };
const SceneId kTarget[6] = { SceneId::Intro, SceneId::Story, SceneId::GameMode, SceneId::Help, SceneId::Donate, SceneId::Quit };
}

MenuScene::MenuScene(Game& g) : Scene(g, SceneId::Menu) {
	g.resize(cfg::kMenuW, cfg::kMenuH);
	if (!Mix_PlayingMusic()) {
		Mix_VolumeMusic(32);
		Mix_PlayMusic(g.menuMusic(), -1);
	}
	SDL_Renderer* r = g.renderer();
	bg_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\ChooseBG.bmp");
	const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\crazy-pixel.ttf", 50);
	items_.reserve(6);
	for (int i = 0; i < 6; ++i) items_.emplace_back(r, font.get(), kLabels[i], kAt[i], color::black, color::red);
	if (!bg_ || !items_[0].ok()) {
		g.fatal("Missing menu resources (see console)");
		go(SceneId::Quit);
	}
}

MenuScene::~MenuScene() {
	if (next_ == SceneId::Intro) Mix_HaltMusic();  // menu.c:66
}

void MenuScene::handleEvent(const SDL_Event& e) {
	if (e.type == SDL_MOUSEMOTION) {
		for (Button& b : items_) b.hover(e.motion.x, e.motion.y);
	}
	else if (e.type == SDL_MOUSEBUTTONDOWN) {
		for (int i = 0; i < 6; ++i) {
			if (items_[i].hit(e.button.x, e.button.y)) {
				Mix_PlayChannel(-1, game_.click(), 0);
				go(kTarget[i]);
			}
		}
	}
	else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
		go(SceneId::Quit);
	}
}

void MenuScene::render(SDL_Renderer* r) {
	SDL_RenderCopy(r, bg_.get(), nullptr, nullptr);
	for (const Button& b : items_) b.draw(r);
}
}
```
Lưu ý so với C: bản C cho mục hover = mục cuối cùng chứa chuột; các rect không chồng nhau nên `hover()` từng nút cho kết quả như nhau.

**`StoryScene.cpp`** — port `story.c`:
```cpp
#include "scenes/Scenes.h"

namespace jr {
namespace {
constexpr SDL_Rect kRight{ 714, 481, 73, 55 }, kLeft{ 206, 480, 74, 55 }, kScreen{ -70, 0, 1110, 668 };
bool inside(const SDL_Rect& r, int x, int y) { return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h; }
}

StoryScene::StoryScene(Game& g) : Scene(g, SceneId::Story) {
	SDL_Renderer* r = g.renderer();
	page_[0] = sdl::loadTexture(r, "Resource\\Menu\\Story\\story1.bmp");
	page_[1] = sdl::loadTexture(r, "Resource\\Menu\\Story\\story2.bmp");
	red_ = sdl::loadTexture(r, "Resource\\Menu\\Story\\red.bmp");
	green_ = sdl::loadTexture(r, "Resource\\Menu\\Story\\green.bmp");
	const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
	back_ = std::make_unique<Button>(r, font.get(), "Return", SDL_Point{ 450, 503 }, color::black, color::aqua);
	if (!page_[0] || !page_[1] || !red_ || !green_ || !back_->ok()) {
		g.fatal("Missing story resources (see console)");
		go(SceneId::Quit);
	}
}

void StoryScene::handleEvent(const SDL_Event& e) {
	if (e.type == SDL_MOUSEMOTION) {
		hotRight_ = inside(kRight, e.motion.x, e.motion.y);
		hotLeft_ = inside(kLeft, e.motion.x, e.motion.y);
		back_->hover(e.motion.x, e.motion.y);
	}
	else if (e.type == SDL_MOUSEBUTTONDOWN) {
		const int x = e.button.x, y = e.button.y;
		if (inside(kRight, x, y)) { Mix_PlayChannel(-1, game_.click(), 0); current_ = 1; }
		if (inside(kLeft, x, y)) { Mix_PlayChannel(-1, game_.click(), 0); current_ = 0; }
		if (back_->hit(x, y)) { Mix_PlayChannel(-1, game_.click(), 0); go(SceneId::Menu); }
	}
	else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
		go(SceneId::Menu);
	}
}

void StoryScene::render(SDL_Renderer* r) {
	SDL_RenderCopy(r, page_[current_].get(), nullptr, &kScreen);
	if (current_ == 0) {
		SDL_RenderCopyEx(r, hotRight_ ? red_.get() : green_.get(), nullptr, &kRight, 0, nullptr, hotRight_ ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
	}
	else {
		SDL_RenderCopyEx(r, hotLeft_ ? red_.get() : green_.get(), nullptr, &kLeft, 0, nullptr, hotLeft_ ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
	}
	back_->draw(r);
}
}
```

**`HelpScene.cpp`** — port `help.c` (cùng tọa độ, cùng thứ tự vẽ):
```cpp
#include "scenes/Scenes.h"

namespace jr {
HelpScene::HelpScene(Game& g) : Scene(g, SceneId::Help) {
	SDL_Renderer* r = g.renderer();
	bg_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\help.bmp");
	fire_ = sdl::loadTexture(r, "Resource\\Menu\\traps\\fire.bmp");
	saw_ = sdl::loadTexture(r, "Resource\\Menu\\traps\\saw.bmp");
	ground_ = sdl::loadTexture(r, "Resource\\Menu\\traps\\ground.bmp");
	lightning_ = sdl::loadTexture(r, "Resource\\Menu\\traps\\lightning.bmp");
	ceiling_ = sdl::loadTexture(r, "Resource\\Menu\\traps\\ceiling.bmp");
	sdl::loadTextureSeq(r, health_, "Resource\\Menu\\item\\health%d.bmp", true);
	sdl::loadTextureSeq(r, good_, "Resource\\Menu\\item\\good%d.bmp", true);
	sdl::loadTextureSeq(r, running_, "Resource\\Menu\\running\\run%d.bmp", false);
	sdl::loadTextureSeq(r, jumping_, "Resource\\Menu\\jump\\jump%d.bmp", false);
	const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
	back_ = std::make_unique<Button>(r, font.get(), "Return", SDL_Point{ 446, 503 }, color::black, color::aqua);
	if (!bg_ || !back_->ok()) {
		g.fatal("Missing help resources (see console)");
		go(SceneId::Quit);
	}
}

void HelpScene::handleEvent(const SDL_Event& e) {
	if (e.type == SDL_MOUSEMOTION) back_->hover(e.motion.x, e.motion.y);
	else if (e.type == SDL_MOUSEBUTTONDOWN && back_->hit(e.button.x, e.button.y)) {
		Mix_PlayChannel(-1, game_.click(), 0);
		go(SceneId::Menu);
	}
	else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) go(SceneId::Menu);
}

void HelpScene::render(SDL_Renderer* r) {
	static constexpr SDL_Rect health{ 550, 302, 40, 40 }, good{ 460, 365, 40, 40 }, running{ 500, 130, 35, 65 },
		jumping{ 430, 214, 35, 65 }, ceiling{ 610, 410, 60, 50 }, fire{ 670, 370, 55, 90 }, saw{ 720, 410, 50, 50 },
		ground{ 750, 410, 70, 50 }, lightning{ 793, 400, 70, 60 };
	const Uint64 t = SDL_GetTicks64();
	SDL_RenderCopy(r, bg_.get(), nullptr, nullptr);
	back_->draw(r);
	SDL_RenderCopy(r, lightning_.get(), nullptr, &lightning);
	SDL_RenderCopy(r, ground_.get(), nullptr, &ground);
	SDL_RenderCopy(r, fire_.get(), nullptr, &fire);
	SDL_RenderCopy(r, saw_.get(), nullptr, &saw);
	SDL_RenderCopy(r, health_[t / 120 % 7].get(), nullptr, &health);
	SDL_RenderCopy(r, good_[t / 120 % 8].get(), nullptr, &good);
	SDL_RenderCopy(r, running_[t / 120 % 8].get(), nullptr, &running);
	SDL_RenderCopy(r, jumping_[t / 200 % 5].get(), nullptr, &jumping);
	SDL_RenderCopy(r, ceiling_.get(), nullptr, &ceiling);
}
}
```

**`DonateScene.cpp`** — port `donate.c`:
```cpp
#include "scenes/Scenes.h"

namespace jr {
DonateScene::DonateScene(Game& g) : Scene(g, SceneId::Donate) {
	SDL_Renderer* r = g.renderer();
	bg_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\donate.bmp");
	qr_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\QR.bmp");
	logo_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\momologo.bmp");
	{
		const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
		back_ = std::make_unique<Button>(r, font.get(), "Return", SDL_Point{ 447, 440 }, color::black, color::aqua);
	}
	const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\Kanit-MediumItalic.ttf", 30);
	thanks_ = sdl::renderText(r, font.get(), "^^Thank You For Your Donation^^", SDL_Color{ 0, 255, 0, 255 });
	thankRect_ = sdl::sized(thanks_.get(), 247, 200);
	if (!bg_ || !back_->ok()) {
		g.fatal("Missing donate resources (see console)");
		go(SceneId::Quit);
	}
}

void DonateScene::handleEvent(const SDL_Event& e) {
	if (e.type == SDL_MOUSEMOTION) back_->hover(e.motion.x, e.motion.y);
	else if (e.type == SDL_MOUSEBUTTONDOWN && back_->hit(e.button.x, e.button.y)) {
		Mix_PlayChannel(-1, game_.click(), 0);
		go(SceneId::Menu);
	}
	else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) go(SceneId::Menu);
}

void DonateScene::render(SDL_Renderer* r) {
	static constexpr SDL_Rect qr{ 398, 255, 170, 170 }, logo{ 328, 300, 60, 60 };
	SDL_RenderCopy(r, bg_.get(), nullptr, nullptr);
	SDL_RenderCopy(r, qr_.get(), nullptr, &qr);
	SDL_RenderCopy(r, logo_.get(), nullptr, &logo);
	SDL_RenderCopy(r, thanks_.get(), nullptr, &thankRect_);
	back_->draw(r);
}
}
```

**`GameModeScene.cpp`** — port `gamemode.c`:
```cpp
#include "scenes/Scenes.h"

namespace jr {
namespace {
const char* const kModes[3] = { "Easy", "Medium", "Hard" };
constexpr int kDifficulty[3] = { cfg::kDifficultyEasy, cfg::kDifficultyMedium, cfg::kDifficultyHard };
constexpr SDL_Color kModeColor[3] = { { 0, 255, 0, 255 }, { 255, 200, 0, 255 }, { 255, 0, 0, 255 } };
constexpr int kModeX[3] = { 435, 415, 435 };
}

GameModeScene::GameModeScene(Game& g) : Scene(g, SceneId::GameMode) {
	SDL_Renderer* r = g.renderer();
	bg_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\gamemode.bmp");
	trick_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\trick3.bmp");
	{
		const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\Kanit-Light.ttf", 24);
		back_ = std::make_unique<Button>(r, font.get(), "Return", SDL_Point{ 446, 439 }, color::black, color::aqua);
	}
	{
		const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\Kanit-Medium.ttf", 60);
		left_ = std::make_unique<Button>(r, font.get(), "<<", SDL_Point{ 310, 290 }, color::black, color::red);
		right_ = std::make_unique<Button>(r, font.get(), ">>", SDL_Point{ 595, 290 }, color::black, color::red);
	}
	{
		const sdl::FontPtr font = sdl::loadFont("Resource\\Fonts\\crazy-pixel.ttf", 100);
		title_ = sdl::renderText(r, font.get(), "Game Mode", SDL_Color{ 0, 200, 255, 255 });
		titleRect_ = sdl::sized(title_.get(), 350, 130);
	}
	modeFont_ = sdl::loadFont("Resource\\Fonts\\crazy-pixel.ttf", 80);
	refreshMode();
	if (!bg_ || !back_->ok() || !modeFont_) {
		g.fatal("Missing game mode resources (see console)");
		go(SceneId::Quit);
	}
}

void GameModeScene::refreshMode() {
	const int i = game_.settings().modeIndex;
	mode_ = sdl::renderText(game_.renderer(), modeFont_.get(), kModes[i], kModeColor[i]);
}

void GameModeScene::handleEvent(const SDL_Event& e) {
	if (e.type == SDL_MOUSEMOTION) {
		back_->hover(e.motion.x, e.motion.y);
		left_->hover(e.motion.x, e.motion.y);
		right_->hover(e.motion.x, e.motion.y);
	}
	else if (e.type == SDL_MOUSEBUTTONDOWN) {
		const int x = e.button.x, y = e.button.y;
		if (back_->hit(x, y)) {
			Mix_PlayChannel(-1, game_.click(), 0);
			go(SceneId::Menu);
		}
		const int step = left_->hit(x, y) ? 2 : right_->hit(x, y) ? 1 : 0;  // 2 == -1 mod 3
		if (step) {
			Settings& s = game_.settings();
			s.modeIndex = (s.modeIndex + step) % 3;
			s.difficulty = kDifficulty[s.modeIndex];
			refreshMode();
		}
	}
	else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
		go(SceneId::Menu);
	}
}

void GameModeScene::render(SDL_Renderer* r) {
	static constexpr SDL_Rect trick{ 390, 300, 190, 70 };
	const SDL_Rect modeRect = sdl::sized(mode_.get(), kModeX[game_.settings().modeIndex], 265);
	SDL_RenderCopy(r, bg_.get(), nullptr, nullptr);
	SDL_RenderCopy(r, trick_.get(), nullptr, &trick);
	back_->draw(r);
	left_->draw(r);
	right_->draw(r);
	SDL_RenderCopy(r, title_.get(), nullptr, &titleRect_);
	SDL_RenderCopy(r, mode_.get(), nullptr, &modeRect);
}
}
```

### Task 2.4: `IntroScene.cpp` — port `intro.c`, giữ vòng lặp

```cpp
#include "scenes/Scenes.h"
#include <cstring>

namespace jr {
namespace {
const char* const kStory[7] = {
	"\"...In 2016, Chris, who wanted to cure his beloved sister Claire's late-stage cancer, set out for the",
	"forest after obtaining the location from a lucky explorer who survived the forest. He hoped to find the",
	"treasure in the legend and use its power to save his sister. He crossed through dangerous creatures and",
	"reached the relic. He advanced deep into the relic, trying to hide and find a way to defeat the Golem.",
	"finally. The Golem chased after him and used its power to trap him in an endless loop of being pursued",
	"until death. Chris, with no weapons except for the sword he picked up from the relic, had to break the",
	"loop and defeat the Golem to reach the treasure.\""
};
}

IntroScene::IntroScene(Game& g) : Scene(g, SceneId::Intro) {
	g.resize(cfg::kWindowW, cfg::kWindowH);
	SDL_Renderer* r = g.renderer();
	typing_ = sdl::loadMusic("Resource\\Sound\\Sound Effects\\typing.mp3");
	Mix_PlayMusic(typing_.get(), -1);
	Mix_VolumeMusic(64);
	{
		const sdl::FontPtr big = sdl::loadFont("Resource\\Fonts\\crazy-pixel.ttf", 80);
		skip_ = sdl::renderText(r, big.get(), ">> Press [SPACE] to skip <<", color::white);
	}
	font_ = sdl::loadFont("Resource\\Fonts\\Kanit-Light.ttf", 20);
	black_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\bg.bmp");
	whitebar_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\whitebar.bmp");
	blackbar_ = sdl::loadTexture(r, "Resource\\Menu\\LoginImage\\blackbar.bmp");
	if (!font_ || !black_ || !whitebar_ || !blackbar_) {
		g.fatal("Missing intro resources (see console)");
		go(SceneId::Quit);
	}
}

IntroScene::~IntroScene() { Mix_HaltMusic(); }  // intro.c:86, before typing_ is freed

SceneId IntroScene::run() {
	SDL_Renderer* r = game_.renderer();
	const SDL_Rect skipRect = sdl::sized(skip_.get(), 290, 500);
	SDL_Rect lineRect{ 120, 76, 0, 0 }, underline{ 100, 100, 15, 5 }, blackRect{ 0, 100, 1200, 800 };
	int charNumber = 0, lineNumber = 0;
	SDL_RenderClear(r);
	while (running()) {
		SDL_Event e;
		while (running() && SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) go(SceneId::Quit);
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) go(SceneId::Menu);
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) go(SceneId::Gameplay);
		}
		if (!running()) break;
		if (charNumber >= static_cast<int>(std::strlen(kStory[lineNumber]))) {
			if (++lineNumber >= 7) {
				go(SceneId::Gameplay);
				break;
			}
			charNumber = 0;
			underline.y += 40;
			underline.x = 100;
			blackRect.y += 40;
			lineRect.y += 40;
		}
		charNumber += 5;
		char prefix[150];
		SDL_strlcpy(prefix, kStory[lineNumber], SDL_min(sizeof prefix, static_cast<size_t>(charNumber) + 1));
		line_ = sdl::renderText(r, font_.get(), prefix, color::white);
		lineRect = sdl::sized(line_.get(), lineRect.x, lineRect.y);
		underline.x += 44;

		SDL_Delay(100);
		SDL_RenderCopy(r, black_.get(), nullptr, &blackRect);
		SDL_RenderCopy(r, line_.get(), nullptr, &lineRect);
		SDL_RenderCopy(r, skip_.get(), nullptr, &skipRect);
		SDL_RenderPresent(r);
		SDL_Delay(60);
		SDL_RenderCopy(r, underline.x >= 1000 ? blackbar_.get() : whitebar_.get(), nullptr, &underline);
		SDL_RenderPresent(r);
		game_.tick();
	}
	return next_;
}
}
```

### Task 2.5: `GameplayScene.cpp`

```cpp
#include "scenes/Scenes.h"

namespace jr {
GameplayScene::GameplayScene(Game& g) : Scene(g, SceneId::Gameplay) {
	g.resize(cfg::kWindowW, cfg::kWindowH);
	if (!assets_.load(g.renderer())) {
		g.fatal("Missing game resources (see console)\nReturning to menu.");
		go(SceneId::Menu);
		return;
	}
	std::string error;
	if (!loadCourse("Resource\\Levels", course_, error)) {
		g.fatal(error + "\nReturning to menu.");
		go(SceneId::Menu);
		return;
	}
	world_ = std::make_unique<World>(assets_, course_, g.settings().difficulty);
	world_->reset();
}

GameplayScene::~GameplayScene() {  // gameplay.c:90-91
	Mix_HaltChannel(-1);
	Mix_HaltMusic();
}

SceneId GameplayScene::run() {
	Uint64 last = SDL_GetTicks64();
	while (running()) {
		const Uint64 now = SDL_GetTicks64();
		const float dt = static_cast<float>(now - last) / 1000.0f;
		last = now;
		const Uint8* k = SDL_GetKeyboardState(nullptr);  // before polling, as in C
		world_->input({ k[SDL_SCANCODE_LEFT] || k[SDL_SCANCODE_A], k[SDL_SCANCODE_RIGHT] || k[SDL_SCANCODE_D],
			k[SDL_SCANCODE_DOWN] || k[SDL_SCANCODE_S] });
		SDL_Event e;
		while (running() && SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				go(SceneId::Quit);
			}
			else if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
				case SDLK_SPACE:
				case SDLK_UP:
				case SDLK_w: world_->jump(); break;
				case SDLK_r:
					if (world_->overReady()) {
						Mix_HaltMusic();
						world_->reset();
					}
					break;
				case SDLK_x:
					if (world_->overReady()) go(SceneId::Menu);
					break;
				default: break;
				}
			}
		}
		if (!running()) break;
		world_->step(dt);
		world_->render(game_.renderer());
		game_.tick();
	}
	return next_;
}
}
```

### Task 2.6: `main.cpp`, xóa C, cập nhật project

**Step 1: `src/main.cpp`**
```cpp
#include "core/Game.h"

int main(int, char*[]) {  // this exact signature: SDL2main.lib expects SDL_main(int, char**)
	jr::Game game;
	return game.ok() ? game.run() : 1;
}
```

**Step 2: Xóa C + công cụ chỉ dùng cho GĐ1** (cùng commit với `main.cpp`):
```bash
git rm -q src/main.c src/core/app.c src/core/app.h src/core/res.c src/core/res.h \
  src/scenes/*.c src/scenes/scene.h \
  src/game/assets.c src/game/physics.c src/game/collision.c src/game/map.c src/game/levels.c src/game/render.c \
  src/game/game.h src/game/gamestate.h src/game/defs.h
git rm -rq tools/parity
```

**Step 3: `.vcxproj`:** ItemGroup ClCompile chỉ còn các `.cpp` (bỏ metadata `WarningLevel` theo file); đặt `<WarningLevel>Level4</WarningLevel>` ở ItemDefinitionGroup Debug|x64 và Release|x64. Thêm: `src\main.cpp`, `src\core\Game.cpp`, `src\scenes\Scene.cpp`, `MenuScene.cpp`, `StoryScene.cpp`, `HelpScene.cpp`, `DonateScene.cpp`, `GameModeScene.cpp`, `IntroScene.cpp`, `GameplayScene.cpp`, `src\ui\Button.cpp`; ClInclude `src\core\Game.h`, `src\scenes\Scene.h`, `src\scenes\Scenes.h`, `src\ui\Button.h`. Filters: thêm `Source Files\ui`, bỏ mục `.c`.

**Step 4: Kiểm tra**
```bash
git ls-files "src/*.c" "src/*/*.c" | wc -l        # Expected: 0
grep -rnE "SDL_GetTicks\(" src/game               # Expected: không có
grep -rnE "(=|return)\s*new\b|\bdelete\s+[a-z_]|malloc\(|\bfree\(" src  # Expected: không có
grep -rnE "^static .*(Ptr|World|GameAssets)" src  # Expected: không có handle RAII static
```

**Step 5:** Build Debug + Release: 0 lỗi, **0 warning** (toàn project ở /W4).

**Step 6: Commit**
```bash
git add -A src "Jungle Run.vcxproj" "Jungle Run.vcxproj.filters" tools
git commit -m "refactor: C++ application layer (Game, Scene, Button, scenes); remove C sources"
```

### Task 2.7: Bàn giao checklist runtime GĐ2

Gửi người dùng checklist mục 5 của thiết kế (C mục 6 + 1–4), nhấn mạnh:
- Mốc bộ đếm = lần đầu vào menu của bản C++ này.
- Lỗi file level: message box chỉ hiện **sau** intro (hết chữ hoặc Space).
- So nhịp bẫy/golem/key với bản C (đã khớp ở kiểm tra song song cho logic; mục này kiểm phần vẽ/âm thanh).

---

## GĐ3 — Tài liệu

### Task 3.1: README

Thay mục “Source layout” và thêm:
- **Source layout:** `src/core` (Game, Sdl), `src/ui` (Button), `src/scenes` (Scene + 7 scene), `src/game` (World, Level, Player, entities, LevelLoader).
- **Level format:** đường dẫn `Resource/Levels`, cú pháp `course.txt` (`ground` / `map` / `onkey` / `finish`), lưới 16×24, legend nằm đầu mỗi `lvNN.txt`, thông báo lỗi trông thế nào.
- **Add a trap:** 4 bước từ thiết kế §4.4 (“Công thức thêm một bẫy mới”).
- **Debug counters:** giữ nguyên, ghi rõ mốc là lần đầu vào menu của cùng bản build.

### Task 3.2: Rà soát 4.5b và chốt

Đối chiếu từng mục “đặc điểm Player phải giữ” (thiết kế §4.5b) với code C++ (`Player.cpp`, `Level.cpp`, `World.cpp`), ghi kết quả vào thiết kế mục 8 (dòng “Rà soát 4.5b: đạt”). Cập nhật trạng thái thiết kế: `đã triển khai, chờ checklist runtime`.

```bash
git add README.md docs/plans
git commit -m "docs: C++ source layout, level format, adding traps"
```
