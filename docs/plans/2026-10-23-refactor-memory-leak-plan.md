# Jungle Run — Kế hoạch triển khai refactor & sửa memory leak

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Hết rò rỉ bộ nhớ (RAM phẳng khi chơi lâu) và chuyển game sang 1 window/renderer + vòng lặp scene, code trong `src/` chia module.

**Architecture:** Mỗi màn hình là `SceneId xxx_run(App*)` có vòng lặp riêng, trả về màn kế tiếp; `main` điều phối bằng `switch`. Mọi tạo/hủy tài nguyên đi qua `core/res.c` (có bộ đếm ở bản Debug). Gameplay nạp tài nguyên 1 lần khi vào, Retry chỉ reset state.

**Tech Stack:** C (MSVC), SDL2 2.26 + SDL2_image 2.6 + SDL2_ttf 2.20 + SDL2_mixer 2.6, Visual Studio `.vcxproj` x64.

**Thiết kế gốc:** [2026-10-23-refactor-memory-leak-design.md](2026-10-23-refactor-memory-leak-design.md)

---

## Quy ước chung

**Không chạy game trên máy dev.** Kiểm chứng ở máy dev = biên dịch. Không có test framework C trong repo; “test” của từng task là compile file đó, “test” cuối là build cả project + checklist runtime trong thiết kế (mục 6) do người dùng chạy.

**Lệnh build** (máy dev có MSBuild 18 với toolset v145; project khai báo v143 nên override trên dòng lệnh, không sửa project):

```bash
MSB="/c/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/MSBuild/Current/Bin/MSBuild.exe"
# build cả project
"$MSB" "Jungle Run.vcxproj" -p:Configuration=Debug -p:Platform=x64 -p:PlatformToolset=v145 -nologo -v:minimal
# compile 1 file
"$MSB" "Jungle Run.vcxproj" -t:ClCompile "-p:SelectedFiles=src\core\res.c" -p:Configuration=Debug -p:Platform=x64 -p:PlatformToolset=v145 -nologo -v:minimal
# đếm warning
... 2>&1 | grep -oE "warning C[0-9]+" | sort | uniq -c
```

Mốc trước refactor: Debug x64 build được, **98 warning** (C4133×33, C4244×27, C4047×13, C4024×10, C4267×6, C4101×6, C4098×2, C4715×1). Release x64 **lỗi** (thiếu include/lib SDL). Mục tiêu: 0 lỗi cả hai cấu hình, warning < 98, không có warning nào trong `src/core` và `src/scenes`.

Đường dẫn tài nguyên giữ dạng `"Resource\\..."` tương đối với thư mục gốc (working dir khi chạy).

## Sai khác so với thiết kế (cần người dùng duyệt)

1. `core/text.c` + kiểu `Label` → gộp thành `res_text()` trong `res.c` và 1 hàm `static hud_label()` trong `game/render.c`. Ít file hơn, cùng tác dụng.
2. Các màn menu (menu, story, help, donate, gamemode) **vẽ lại toàn bộ mỗi frame** thay vì vẽ chắp vá theo sự kiện như hiện tại. Cùng ảnh, cùng tọa độ; code ngắn hơn nhiều và không dựa vào back-buffer giữ nội dung sau `SDL_RenderPresent` (hành vi không được SDL đảm bảo). Màn intro (chữ chạy) **giữ** cách vẽ chắp vá cũ vì nó cố ý cộng dồn các dòng.
3. Mũi tên `<<` `>>` ở Game Mode dùng font 60pt cả lúc hover (bản cũ đổi sang 40pt bị kéo giãn sau lần rê chuột đầu tiên).
4. `GameState` vẫn giữ con trỏ `renderer` (mượn của `App`, không sở hữu) để khỏi sửa chữ ký mọi hàm vẽ.
5. Gộp giai đoạn 2+3 của thiết kế: game module và scenes phải đổi cùng lúc mới build được (scene cũ gọi thẳng `gamePlay()`). Mỗi task trong GĐ2 kiểm tra bằng compile riêng file đó; build cả project ở cuối GĐ2.
6. Thêm bug sửa (phát hiện khi lập kế hoạch): `lineTexture[2]` bị ghi tới chỉ số 6 (tràn bộ đệm) và `strlen(story[7])` đọc ngoài mảng ở intro; R/X ở màn kết thúc bị `processEvents()` nuốt mất sự kiện; vào Game Mode luôn reset độ khó về Easy dù nhãn hiện lựa chọn cũ; `drawTilesMap` nhận `Map` theo giá trị (copy ~23KB mỗi ô mỗi frame) → đổi sang `const Map*`.
7. Hiệu ứng trúng bẫy: nhân vật nhấp nháy đỏ/thường mỗi 100ms trong lúc bất tử (bản cũ nháy do gọi `SDL_RenderPresent` giữa frame).

---

## GĐ0 — Dọn repo

### Task 0.1: Tạo branch

**Step 1:**
```bash
git checkout -b refactor/scene-loop-memleak
```
Expected: `Switched to a new branch 'refactor/scene-loop-memleak'`

### Task 0.2: `.gitignore` + gỡ build artifacts khỏi git

**Files:** Create `.gitignore`

**Step 1: Viết `.gitignore`**
```gitignore
# Visual Studio
.vs/
x64/
Debug/
Release/
*.obj
*.pdb
*.ilk
*.idb
*.ipch
*.tlog
*.lastbuildstate
# macOS
.DS_Store
```
(`Jungle Run.vcxproj.user` vẫn được track: chứa PATH tới DLL cho debugger.)

**Step 2: Gỡ khỏi index (file trên đĩa giữ nguyên)**
```bash
git rm -r --cached --quiet .vs x64 Debug
git ls-files "*.DS_Store" | xargs -d '\n' git rm --cached --quiet
```

**Step 3: Kiểm tra**
```bash
git ls-files | grep -cE "^(\.vs|x64|Debug)/"   # Expected: 0
ls .vs x64 Debug >/dev/null && echo still-on-disk  # Expected: still-on-disk
```

**Step 4: Commit**
```bash
git add .gitignore
git commit -m "chore: add .gitignore and untrack build artifacts"
```

---

## GĐ1 — Core (thêm mới, chưa dùng; code cũ vẫn build)

### Task 1.1: `src/core/res.h` + `res.c`

**Files:** Create `src/core/res.h`, `src/core/res.c`

**Step 1: `src/core/res.h`**
```c
#ifndef RES_H
#define RES_H
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

// Every texture/font/sound in the game is created and destroyed through these,
// so the Debug build can count what is alive.
SDL_Texture* res_texture(SDL_Renderer* r, const char* path, int keyBlack);
void res_texture_seq(SDL_Renderer* r, SDL_Texture* out[], int n, const char* fmt, int keyBlack); // fmt has %d, 1-based
SDL_Texture* res_text(SDL_Renderer* r, TTF_Font* font, const char* text, SDL_Color color);
void res_destroy_texture(SDL_Texture** t);
void res_destroy_seq(SDL_Texture* t[], int n);

TTF_Font* res_font(const char* path, int size);
void res_close_font(TTF_Font** f);

Mix_Chunk* res_wav(const char* path, int volume); // volume < 0: keep default
void res_free_wav(Mix_Chunk** c);
Mix_Music* res_mus(const char* path);
void res_free_mus(Mix_Music** m);

// Debug: "tex 42 font 3 wav 7 mus 2 | sdl_alloc 1234". Release: "".
void res_counts(char* buf, size_t len);
#endif
```

**Step 2: `src/core/res.c`**
```c
#include "core/res.h"
#include <SDL_image.h>

#ifdef _DEBUG
enum { K_TEX, K_FONT, K_WAV, K_MUS, K_COUNT };
static int live[K_COUNT];
#define TRACK(k, d) (live[k] += (d))
#else
#define TRACK(k, d) ((void)0)
#endif

static SDL_Texture* from_surface(SDL_Renderer* r, SDL_Surface* s, const char* what, int keyBlack) {
	if (!s) {
		SDL_Log("load fail: %s: %s", what, SDL_GetError());
		return NULL;
	}
	if (keyBlack) {
		SDL_SetColorKey(s, SDL_TRUE, SDL_MapRGB(s->format, 0, 0, 0));
	}
	SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
	SDL_FreeSurface(s);
	if (!t) {
		SDL_Log("texture fail: %s: %s", what, SDL_GetError());
		return NULL;
	}
	TRACK(K_TEX, +1);
	return t;
}

SDL_Texture* res_texture(SDL_Renderer* r, const char* path, int keyBlack) {
	return from_surface(r, IMG_Load(path), path, keyBlack);
}

void res_texture_seq(SDL_Renderer* r, SDL_Texture* out[], int n, const char* fmt, int keyBlack) {
	char path[256];
	for (int i = 0; i < n; i++) {
		SDL_snprintf(path, sizeof path, fmt, i + 1);
		out[i] = res_texture(r, path, keyBlack);
	}
}

SDL_Texture* res_text(SDL_Renderer* r, TTF_Font* font, const char* text, SDL_Color color) {
	return from_surface(r, font ? TTF_RenderText_Solid(font, text, color) : NULL, text, 0);
}

void res_destroy_texture(SDL_Texture** t) {
	if (*t) {
		SDL_DestroyTexture(*t);
		TRACK(K_TEX, -1);
		*t = NULL;
	}
}

void res_destroy_seq(SDL_Texture* t[], int n) {
	for (int i = 0; i < n; i++) {
		res_destroy_texture(&t[i]);
	}
}

TTF_Font* res_font(const char* path, int size) {
	TTF_Font* f = TTF_OpenFont(path, size);
	if (!f) {
		SDL_Log("font fail: %s: %s", path, SDL_GetError());
		return NULL;
	}
	TRACK(K_FONT, +1);
	return f;
}

void res_close_font(TTF_Font** f) {
	if (*f) {
		TTF_CloseFont(*f);
		TRACK(K_FONT, -1);
		*f = NULL;
	}
}

Mix_Chunk* res_wav(const char* path, int volume) {
	Mix_Chunk* c = Mix_LoadWAV(path);
	if (!c) {
		SDL_Log("sound fail: %s: %s", path, SDL_GetError());
		return NULL;
	}
	if (volume >= 0) {
		Mix_VolumeChunk(c, volume);
	}
	TRACK(K_WAV, +1);
	return c;
}

void res_free_wav(Mix_Chunk** c) {
	if (*c) {
		Mix_FreeChunk(*c);
		TRACK(K_WAV, -1);
		*c = NULL;
	}
}

Mix_Music* res_mus(const char* path) {
	Mix_Music* m = Mix_LoadMUS(path);
	if (!m) {
		SDL_Log("music fail: %s: %s", path, SDL_GetError());
		return NULL;
	}
	TRACK(K_MUS, +1);
	return m;
}

void res_free_mus(Mix_Music** m) {
	if (*m) {
		Mix_FreeMusic(*m);
		TRACK(K_MUS, -1);
		*m = NULL;
	}
}

void res_counts(char* buf, size_t len) {
#ifdef _DEBUG
	SDL_snprintf(buf, len, "tex %d font %d wav %d mus %d | sdl_alloc %d",
		live[K_TEX], live[K_FONT], live[K_WAV], live[K_MUS], SDL_GetNumAllocations());
#else
	SDL_snprintf(buf, len, "");
#endif
}
```

Ghi chú: `IMG_Load` đọc được cả BMP nên một hàm `res_texture` thay cho cả `SDL_LoadBMP` lẫn `IMG_Load`.

### Task 1.2: `src/core/app.h` + `app.c`

**Files:** Create `src/core/app.h`, `src/core/app.c`

**Step 1: `src/core/app.h`**
```c
#ifndef APP_H
#define APP_H
#include <SDL.h>
#include <SDL_mixer.h>

#define MENU_W 960
#define MENU_H 668

typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;
	Mix_Chunk* click;      // menu click sound, shared by all menu scenes
	Mix_Music* menuMusic;  // keeps playing across menu scenes
	int difficulty;        // DIFFICULTY_EASY / MEDIUM / HARD
	int modeIndex;         // 0 Easy, 1 Medium, 2 Hard (Game Mode label)
} App;

int app_init(App* app); // 1 = ok
void app_quit(App* app);
void app_resize(App* app, int w, int h);
void app_tick(App* app); // once per loop iteration; Debug: live counts in title every second
void app_fatal(App* app, const char* msg);
#endif
```

**Step 2: `src/core/app.c`**
```c
#include "core/app.h"
#include "core/res.h"
#include "game/defs.h"
#include <SDL_image.h>
#include <SDL_ttf.h>

void app_fatal(App* app, const char* msg) {
	SDL_Log("fatal: %s", msg);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Jungle Run", msg, app ? app->window : NULL);
}

int app_init(App* app) {
	SDL_memset(app, 0, sizeof *app);
	app->difficulty = DIFFICULTY_EASY;
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
		app_fatal(NULL, SDL_GetError());
		return 0;
	}
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	Mix_Init(MIX_INIT_MP3);
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048) != 0) {
		SDL_Log("audio: %s", SDL_GetError()); // play without sound
	}
	app->window = SDL_CreateWindow("Jungle Run", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MENU_W, MENU_H, 0);
	if (!app->window) {
		app_fatal(NULL, SDL_GetError());
		return 0;
	}
	app->renderer = SDL_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!app->renderer) {
		app_fatal(app, SDL_GetError());
		return 0;
	}
	app->click = res_wav("Resource\\Sound\\Sound Effects\\click.wav", -1);
	app->menuMusic = res_mus("Resource\\Sound\\Soundtrack\\MainMenu2.mp3");
	return 1;
}

void app_quit(App* app) {
	Mix_HaltMusic();
	res_free_mus(&app->menuMusic);
	res_free_wav(&app->click);
	if (app->renderer) SDL_DestroyRenderer(app->renderer);
	if (app->window) SDL_DestroyWindow(app->window);
	Mix_CloseAudio();
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void app_resize(App* app, int w, int h) {
	int cw, ch;
	SDL_GetWindowSize(app->window, &cw, &ch);
	if (cw == w && ch == h) return;
	SDL_SetWindowSize(app->window, w, h);
	SDL_SetWindowPosition(app->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void app_tick(App* app) {
#ifdef _DEBUG
	static Uint32 last;
	Uint32 now = SDL_GetTicks();
	if (now - last < 1000) return;
	last = now;
	char counts[128], title[160];
	res_counts(counts, sizeof counts);
	SDL_snprintf(title, sizeof title, "Jungle Run [%s]", counts);
	SDL_SetWindowTitle(app->window, title);
#else
	(void)app;
#endif
}
```

`app.c` include `game/defs.h` → file này chưa tồn tại ở GĐ1. Ở GĐ1 tạo luôn bằng `git mv defs.h src/game/defs.h` **không được** (code cũ include `"defs.h"`). Thay vào đó Task 1.2 dùng `#include "../../defs.h"` tạm; Task 2.1 đổi lại thành `"game/defs.h"`.

### Task 1.3: `.vcxproj` — include dir, cấu hình Release, thêm core

**Files:** Modify `Jungle Run.vcxproj`

**Step 1:** Sau `PropertyGroup` Debug|x64 (dòng 73–77) thêm bản cho Release|x64:
```xml
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <IncludePath>$(ProjectDir)External library\SDL2\include;$(ProjectDir)External library\SDL2_ttf\include;$(ProjectDir)External library\SDL2_image\include;$(ProjectDir)External library\SDL2_mixer\include;$(IncludePath)</IncludePath>
    <LibraryPath>$(ProjectDir)External library\SDL2_image\lib\x64;$(ProjectDir)External library\SDL2\lib\x64;$(ProjectDir)External library\SDL2_ttf\lib\x64;$(ProjectDir)External library\SDL2_mixer\lib\x64;$(LibraryPath)</LibraryPath>
    <IntDir>$(Platform)\$(Configuration)\$(ProjectName)\</IntDir>
  </PropertyGroup>
```

**Step 2:** Trong `ClCompile` của **cả Debug|x64 và Release|x64** thêm:
```xml
      <AdditionalIncludeDirectories>$(ProjectDir)src;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
```
Release|x64 `PreprocessorDefinitions` → `NDEBUG;_CONSOLE;_CRT_SECURE_NO_WARNINGS;%(PreprocessorDefinitions)`.
Release|x64 `Link` thêm dòng `AdditionalDependencies` giống Debug|x64 (dòng 116).

**Step 3:** Thêm vào `ItemGroup` ClCompile: `<ClCompile Include="src\core\app.c" />`, `<ClCompile Include="src\core\res.c" />`; vào ClInclude: `src\core\app.h`, `src\core\res.h`.

**Step 4: Build cả hai cấu hình**
```bash
"$MSB" "Jungle Run.vcxproj" -p:Configuration=Debug   -p:Platform=x64 -p:PlatformToolset=v145 -nologo -v:minimal 2>&1 | grep -E " error |Jungle Run.exe"
"$MSB" "Jungle Run.vcxproj" -p:Configuration=Release -p:Platform=x64 -p:PlatformToolset=v145 -nologo -v:minimal 2>&1 | grep -E " error |Jungle Run.exe"
```
Expected: mỗi lệnh in dòng `Jungle Run.vcxproj -> ...Jungle Run.exe`, không có ` error `. Không có warning nào từ `src\core\`.

**Step 5: Commit**
```bash
git add src/core "Jungle Run.vcxproj"
git commit -m "feat(core): add App and res layer with debug resource counters; fix Release x64 config"
```

---

## GĐ2 — Game module + scenes (build cả project ở cuối GĐ)

### Task 2.1: Di chuyển file game, header gộp, levels

**Files:**
- `git mv`: `defs.h → src/game/defs.h`, `structs.h → src/game/gamestate.h`, `map.c → src/game/map.c`, `collision_handling.c → src/game/collision.c`, `common_functions.c → src/game/physics.c`, `load.c → src/game/assets.c`, `render.c → src/game/render.c`, `main.c → src/main.c`, `menu.c → src/scenes/menu.c`, `help.c → src/scenes/help.c`, `preGameplay.c → src/scenes/intro.c`
- Create: `src/game/levels.c`, `src/game/game.h`
- Delete: `map.h`, `common_functions.h`, `load.h`, `render.h`, `collision_handling.h`, `menu.h`, `help.h`, `preGameplay.h`, `checkAccount.c`, `checkAccount.h`, `user.txt`
- Modify: `src/core/app.c` (include), `Jungle Run.vcxproj`, `Jungle Run.vcxproj.filters`

**Step 1: Tách mảng level ra `levels.c`** (map.h dòng 5–306 là `lv0`…`lv15`)
```bash
mkdir -p src/game src/scenes
{ echo '#include "game/game.h"'; echo; sed -n '5,306p' map.h | sed 's/^static int /int /'; } > src/game/levels.c
grep -c "^int lv" src/game/levels.c   # Expected: 16
```

**Step 2: git mv + git rm** (danh sách ở trên)
```bash
git mv defs.h src/game/defs.h
git mv structs.h src/game/gamestate.h
git mv map.c src/game/map.c
git mv collision_handling.c src/game/collision.c
git mv common_functions.c src/game/physics.c
git mv load.c src/game/assets.c
git mv render.c src/game/render.c
git mv main.c src/main.c
git mv menu.c src/scenes/menu.c
git mv help.c src/scenes/help.c
git mv preGameplay.c src/scenes/intro.c
git rm -q map.h common_functions.h load.h render.h collision_handling.h menu.h help.h preGameplay.h checkAccount.c checkAccount.h user.txt
```
`src/core/app.c`: `#include "../../defs.h"` → `#include "game/defs.h"`.

**Step 3: `src/game/gamestate.h`** — sửa so với `structs.h` cũ:
- guard `STRUCTS` → `GAMESTATE_H`; `#include "defs.h"` → `#include "game/defs.h"`.
- `int mode, difficulty, setDifficulty;` → `int mode, difficulty;`
- Thay khối `// Label` … `// Fonts` (`label_lives`, `label_health_potion`, `TTF_Font* font;`) bằng:
```c
	// HUD text, re-rendered only when the value changes
	SDL_Texture* label_lives;
	SDL_Texture* label_health_potion;
	int cached_lives, cached_potions;
	// Ending screen text, created once per ending
	SDL_Texture* over_title;
	SDL_Texture* over_hint;
	// Fonts
	TTF_Font* hud_font;  // crazy-pixel 48
	TTF_Font* over_font; // Kanit-Light 108
```
- `SDL_Renderer* renderer;` giữ nguyên, thêm comment `// borrowed from App`.

**Step 4: `src/game/game.h`**
```c
#ifndef GAME_H
#define GAME_H
#include "game/gamestate.h"

// levels.c
extern int lv0[MAP_HEIGHT][MAP_WIDTH], lv1[MAP_HEIGHT][MAP_WIDTH], lv2[MAP_HEIGHT][MAP_WIDTH], lv3[MAP_HEIGHT][MAP_WIDTH],
	lv4[MAP_HEIGHT][MAP_WIDTH], lv5[MAP_HEIGHT][MAP_WIDTH], lv6[MAP_HEIGHT][MAP_WIDTH], lv7[MAP_HEIGHT][MAP_WIDTH],
	lv8[MAP_HEIGHT][MAP_WIDTH], lv9[MAP_HEIGHT][MAP_WIDTH], lv10[MAP_HEIGHT][MAP_WIDTH], lv11[MAP_HEIGHT][MAP_WIDTH],
	lv12[MAP_HEIGHT][MAP_WIDTH], lv13[MAP_HEIGHT][MAP_WIDTH], lv14[MAP_HEIGHT][MAP_WIDTH], lv15[MAP_HEIGHT][MAP_WIDTH];

// assets.c
int game_assets_load(GameState* gs); // 1 = every required asset loaded
void game_assets_free(GameState* gs);
void game_reset(GameState* gs);      // new run / retry: state only, loads nothing

// physics.c
void load_player_info(GameState* gameState, float* px, float* py, float* pw, float* ph);
int animation_smoothness(int frame, const int frames);
void add_physics(GameState* gameState);
void detectPlayer(GameState* gameState, Map* map, int row, int col, float dt, float px, float py, float pw, float ph);
void updateGolem(Map* map, int row, int col, float dt);
void updateBoss(GameState* gameState, float dt);
int touchBoss(GameState* gameState);
void immortal_events(GameState* gameState);

// collision.c
void collision_detect_floor(GameState* gameState);
void collision_detect_map(GameState* gameState, Map* map);
void collision_correction(GameState* gameState, float px, float py, float pw, float ph, float bx, float by, float bw, float bh);

// map.c
void loadMap(Map* map, int arr[][MAP_WIDTH], float x, float y);
void drawMap(GameState* gameState, Map* map);
void drawLedges(GameState* gameState);
void drawTilesMap(GameState* gameState, const Map* map, SDL_Rect srcRect, SDL_Texture* texture, int row, int col, double angle, int flip);
void drawBackground(GameState* gameState);
void drawMap_SingleplayerMode(GameState* gameState);
void draw_key_map(GameState* gameState, int ledgeNumber);

// render.c
void do_render(GameState* gameState);
#endif
```

**Step 5: `.vcxproj` — thay 2 ItemGroup ClCompile/ClInclude bằng**
```xml
  <ItemGroup>
    <ClCompile Include="src\main.c" />
    <ClCompile Include="src\core\app.c" />
    <ClCompile Include="src\core\res.c" />
    <ClCompile Include="src\scenes\menu.c" />
    <ClCompile Include="src\scenes\story.c" />
    <ClCompile Include="src\scenes\help.c" />
    <ClCompile Include="src\scenes\donate.c" />
    <ClCompile Include="src\scenes\gamemode.c" />
    <ClCompile Include="src\scenes\intro.c" />
    <ClCompile Include="src\scenes\gameplay.c" />
    <ClCompile Include="src\game\assets.c" />
    <ClCompile Include="src\game\physics.c" />
    <ClCompile Include="src\game\collision.c" />
    <ClCompile Include="src\game\map.c" />
    <ClCompile Include="src\game\levels.c" />
    <ClCompile Include="src\game\render.c" />
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="src\core\app.h" />
    <ClInclude Include="src\core\res.h" />
    <ClInclude Include="src\scenes\scene.h" />
    <ClInclude Include="src\game\defs.h" />
    <ClInclude Include="src\game\gamestate.h" />
    <ClInclude Include="src\game\game.h" />
  </ItemGroup>
```

**Step 6: `Jungle Run.vcxproj.filters`** — ghi đè:
```xml
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <Filter Include="src"><UniqueIdentifier>{5b1f0c1e-3a43-4b8e-9a51-1c2d7f0e6a01}</UniqueIdentifier></Filter>
    <Filter Include="src\core"><UniqueIdentifier>{5b1f0c1e-3a43-4b8e-9a51-1c2d7f0e6a02}</UniqueIdentifier></Filter>
    <Filter Include="src\scenes"><UniqueIdentifier>{5b1f0c1e-3a43-4b8e-9a51-1c2d7f0e6a03}</UniqueIdentifier></Filter>
    <Filter Include="src\game"><UniqueIdentifier>{5b1f0c1e-3a43-4b8e-9a51-1c2d7f0e6a04}</UniqueIdentifier></Filter>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="src\main.c"><Filter>src</Filter></ClCompile>
    <ClCompile Include="src\core\app.c"><Filter>src\core</Filter></ClCompile>
    <ClCompile Include="src\core\res.c"><Filter>src\core</Filter></ClCompile>
    <ClCompile Include="src\scenes\menu.c"><Filter>src\scenes</Filter></ClCompile>
    <ClCompile Include="src\scenes\story.c"><Filter>src\scenes</Filter></ClCompile>
    <ClCompile Include="src\scenes\help.c"><Filter>src\scenes</Filter></ClCompile>
    <ClCompile Include="src\scenes\donate.c"><Filter>src\scenes</Filter></ClCompile>
    <ClCompile Include="src\scenes\gamemode.c"><Filter>src\scenes</Filter></ClCompile>
    <ClCompile Include="src\scenes\intro.c"><Filter>src\scenes</Filter></ClCompile>
    <ClCompile Include="src\scenes\gameplay.c"><Filter>src\scenes</Filter></ClCompile>
    <ClCompile Include="src\game\assets.c"><Filter>src\game</Filter></ClCompile>
    <ClCompile Include="src\game\physics.c"><Filter>src\game</Filter></ClCompile>
    <ClCompile Include="src\game\collision.c"><Filter>src\game</Filter></ClCompile>
    <ClCompile Include="src\game\map.c"><Filter>src\game</Filter></ClCompile>
    <ClCompile Include="src\game\levels.c"><Filter>src\game</Filter></ClCompile>
    <ClCompile Include="src\game\render.c"><Filter>src\game</Filter></ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="src\core\app.h"><Filter>src\core</Filter></ClInclude>
    <ClInclude Include="src\core\res.h"><Filter>src\core</Filter></ClInclude>
    <ClInclude Include="src\scenes\scene.h"><Filter>src\scenes</Filter></ClInclude>
    <ClInclude Include="src\game\defs.h"><Filter>src\game</Filter></ClInclude>
    <ClInclude Include="src\game\gamestate.h"><Filter>src\game</Filter></ClInclude>
    <ClInclude Include="src\game\game.h"><Filter>src\game</Filter></ClInclude>
  </ItemGroup>
</Project>
```

**Step 7: Compile kiểm tra**
```bash
"$MSB" "Jungle Run.vcxproj" -t:ClCompile "-p:SelectedFiles=src\game\levels.c;src\core\app.c" -p:Configuration=Debug -p:Platform=x64 -p:PlatformToolset=v145 -nologo -v:minimal 2>&1 | grep -E " error | warning "
```
Expected: không có output.

### Task 2.2: `src/game/map.c`

**Edits:**
1. Thay 3 dòng include đầu bằng `#include "game/game.h"`.
2. `drawTilesMap`: chữ ký `Map map` → `const Map* map`; thân hàm `map.ledges` → `map->ledges` (3 chỗ trong dòng `dstRect`).
3. Tất cả lời gọi `drawTilesMap(gameState, *map, srcRect, ...)` → `drawTilesMap(gameState, map, srcRect, ...)`:
   `sed -i 's/drawTilesMap(gameState, \*map, /drawTilesMap(gameState, map, /' src/game/map.c`
4. `drawBackground`: xóa 2 dòng `SDL_SetRenderTarget(renderer, ...)`.
5. `drawMap_SingleplayerMode` và `draw_key_map`: `Map* map = &gameState->map;` → `Map* map = gameState->map;`

**Check:** `grep -c "SetRenderTarget\|\*map, srcRect\|&gameState->map" src/game/map.c` → `0`. Compile `src\game\map.c` → không error, không warning C4047/C4133.

### Task 2.3: `src/game/collision.c`

**Edit:** thay `#include "collision_handling.h"`, `"load.h"`, `"map.h"` bằng `#include "game/game.h"`. Compile → không error.

### Task 2.4: `src/game/physics.c`

**Edits:**
1. Include: chỉ giữ `#include "game/game.h"` và `#include <math.h>`.
2. **Xóa** các hàm: `gamePlay`, `processEvents`, `onTrapHit`, `Over`, `clean` (chuyển sang `scenes/gameplay.c` và `game/render.c` ở task sau).
3. **Giữ nguyên thân**: `add_physics`, `animation_smoothness`, `detectPlayer`, `updateGolem`, `updateBoss`, `touchBoss`, `immortal_events`.
4. Chuyển `load_player_info` từ `assets.c` (cũ `load.c` dòng 185–197) sang cuối file này, thân giữ nguyên.

Compile → không error.

### Task 2.5: `src/game/assets.c` — nạp 1 lần / giải phóng / reset

**Ghi đè toàn bộ file:**
```c
#include "game/game.h"
#include "core/res.h"

#define COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))
#define TRAPS "Resource\\Foozle_2DTR0001_Pixel_Trap_Pack\\"

static int all_set(SDL_Texture* const t[], int n) {
	for (int i = 0; i < n; i++) {
		if (!t[i]) return 0;
	}
	return 1;
}

int game_assets_load(GameState* gs) {
	SDL_Renderer* r = gs->renderer;
	static const int scrollSpeed[5] = { PLX_BACKGROUND_1, PLX_BACKGROUND_2, PLX_BACKGROUND_3, PLX_BACKGROUND_4, PLX_BACKGROUND_5 };

	res_texture_seq(r, gs->idle_anim, COUNT(gs->idle_anim), "Resource\\Character\\Idle\\Idle_%d.png", 1);
	res_texture_seq(r, gs->run_anim, COUNT(gs->run_anim), "Resource\\Character\\Run\\Run_%d.png", 1);
	res_texture_seq(r, gs->jump_anim, COUNT(gs->jump_anim), "Resource\\Character\\Jump\\Jump_%d.png", 1);
	gs->golem[0] = res_texture(r, "Resource\\Mecha-stone Golem 0.1\\PNG sheet\\Character_sheet.png", 1);
	for (int i = 0; i < 5; i++) {
		char path[128];
		SDL_snprintf(path, sizeof path, "Resource\\Jungle Asset Pack\\parallax background\\plx-%d.png", i + 1);
		gs->background[i].scrollSpeed = scrollSpeed[i];
		for (int j = 0; j < 3; j++) {
			gs->background[i].layer[j] = res_texture(r, path, 0);
		}
	}
	res_texture_seq(r, gs->platform, 5, "Resource\\Some Bullshit Platform\\Platform %d.png", 1);
	gs->platform[5] = res_texture(r, "Resource\\Jungle Asset Pack\\jungle tileset\\jungle tileset.png", 0);
	res_texture_seq(r, gs->fire_trap, COUNT(gs->fire_trap), TRAPS "Fire Trap\\PNGs\\Fire Trap - Level %d.png", 1);
	res_texture_seq(r, gs->ceiling_trap, COUNT(gs->ceiling_trap), TRAPS "Ceiling Trap\\PNGs\\Ceiling Trap - Level %d.png", 1);
	res_texture_seq(r, gs->saw_trap, COUNT(gs->saw_trap), TRAPS "Saw Trap\\PNGs\\Saw Trap - Level %d.png", 1);
	res_texture_seq(r, gs->lightning_trap, COUNT(gs->lightning_trap), TRAPS "Lightning Trap\\PNGs\\Lightning Trap - Level %d.png", 1);
	gs->items[0] = res_texture(r, "Resource\\platformer items\\animated_items.png", 1);
	gs->sparkling_star[0] = res_texture(r, "Resource\\sparkle\\sparkle.png", 1);

	gs->hud_font = res_font("Resource\\Fonts\\crazy-pixel.ttf", 48);
	gs->over_font = res_font("Resource\\Fonts\\Kanit-Light.ttf", 108);

	SoundEffect* se = &gs->soundEffects;
	se->getDamaged = res_wav("Resource\\Sound\\Sound Effects\\DamageSound.wav", 64);
	se->electricHurt = res_wav("Resource\\Sound\\Sound Effects\\ElectricHurtSound.wav", 64);
	se->ItemPickUp = res_wav("Resource\\Sound\\Sound Effects\\ItemPickupSound.wav", 96);
	se->jump = res_wav("Resource\\Sound\\Sound Effects\\jump.wav", 16);
	se->landing = res_wav("Resource\\Sound\\Sound Effects\\LandingSound.wav", 16);
	se->spike = res_wav("Resource\\Sound\\Sound Effects\\SpikeSound.wav", 32);

	Music* m = &gs->soundTracks;
	m->inGame[0] = res_mus("Resource\\Sound\\Soundtrack\\In Game 3.mp3");
	m->ending[STATUS_STATE_GAMEOVER] = res_mus("Resource\\Sound\\Soundtrack\\Bad Ending.mp3");
	m->ending[STATUS_STATE_TRUE_END] = res_mus("Resource\\Sound\\Soundtrack\\True Ending.mp3");
	m->ending[STATUS_STATE_GOOD_END] = res_mus("Resource\\Sound\\Soundtrack\\Good Ending.mp3");
	Mix_VolumeMusic(32);

	// sounds are optional (game runs silent, as before); images and fonts are not
	int ok = gs->hud_font && gs->over_font
		&& all_set(gs->idle_anim, COUNT(gs->idle_anim)) && all_set(gs->run_anim, COUNT(gs->run_anim))
		&& all_set(gs->jump_anim, COUNT(gs->jump_anim)) && all_set(gs->golem, COUNT(gs->golem))
		&& all_set(gs->platform, COUNT(gs->platform)) && all_set(gs->fire_trap, COUNT(gs->fire_trap))
		&& all_set(gs->ceiling_trap, COUNT(gs->ceiling_trap)) && all_set(gs->saw_trap, COUNT(gs->saw_trap))
		&& all_set(gs->lightning_trap, COUNT(gs->lightning_trap)) && all_set(gs->items, COUNT(gs->items))
		&& all_set(gs->sparkling_star, COUNT(gs->sparkling_star));
	for (int i = 0; i < 5 && ok; i++) {
		ok = all_set(gs->background[i].layer, 3);
	}
	return ok;
}

void game_assets_free(GameState* gs) {
	res_destroy_seq(gs->idle_anim, COUNT(gs->idle_anim));
	res_destroy_seq(gs->run_anim, COUNT(gs->run_anim));
	res_destroy_seq(gs->jump_anim, COUNT(gs->jump_anim));
	res_destroy_seq(gs->golem, COUNT(gs->golem));
	for (int i = 0; i < 5; i++) {
		res_destroy_seq(gs->background[i].layer, 3);
	}
	res_destroy_seq(gs->platform, COUNT(gs->platform));
	res_destroy_seq(gs->fire_trap, COUNT(gs->fire_trap));
	res_destroy_seq(gs->ceiling_trap, COUNT(gs->ceiling_trap));
	res_destroy_seq(gs->saw_trap, COUNT(gs->saw_trap));
	res_destroy_seq(gs->lightning_trap, COUNT(gs->lightning_trap));
	res_destroy_seq(gs->items, COUNT(gs->items));
	res_destroy_seq(gs->sparkling_star, COUNT(gs->sparkling_star));
	res_destroy_texture(&gs->label_lives);
	res_destroy_texture(&gs->label_health_potion);
	res_destroy_texture(&gs->over_title);
	res_destroy_texture(&gs->over_hint);
	res_close_font(&gs->hud_font);
	res_close_font(&gs->over_font);
	SoundEffect* se = &gs->soundEffects;
	res_free_wav(&se->getDamaged);
	res_free_wav(&se->electricHurt);
	res_free_wav(&se->ItemPickUp);
	res_free_wav(&se->jump);
	res_free_wav(&se->landing);
	res_free_wav(&se->spike);
	res_free_mus(&gs->soundTracks.inGame[0]);
	for (int i = 0; i < 3; i++) {
		res_free_mus(&gs->soundTracks.ending[i]);
	}
}

void game_reset(GameState* gs) {
	gs->mode = GAMEMODE_SINGLEPLAYER;
	gs->dt = 0;
	gs->overTiming = 0;
	gs->statusState = STATUS_STATE_GAME;
	gs->isOver = 0;
	gs->player = (Object){ .x = 500, .y = 220 };
	if (gs->difficulty == DIFFICULTY_MEDIUM) gs->player.lives = MAX_HEALTH_MEDIUM;
	else if (gs->difficulty == DIFFICULTY_HARD) gs->player.lives = MAX_HEALTH_HARD;
	else gs->player.lives = MAX_HEALTH_EASY;
	gs->health_potion_counter = 0;
	gs->isKeyObtained = 0;
	gs->flag = 0;
	gs->bossStart = SDL_GetTicks64() / 1000.0f;
	gs->gigaGolem = (Golem){ .pos = { 0, (HEIGHT_WINDOW - HEIGHT_GIGA_GOLEM) / 2.0f } };
	gs->scrollX = 0;
	for (int i = 0; i < 5; i++) {
		gs->background[i].defaultX = 0;
		gs->background[i].previousX = -WIDTH_WINDOW;
		gs->background[i].nextX = WIDTH_WINDOW;
	}
	res_destroy_texture(&gs->over_title); // next ending may have a different title
	res_destroy_texture(&gs->over_hint);
	gs->map->counter = 0;
	drawMap_SingleplayerMode(gs);
}
```

Compile → không error.

### Task 2.6: `src/game/render.c`

**Ghi đè toàn bộ file:**
```c
#include "game/game.h"
#include "core/res.h"

static const SDL_Color WHITE = { 255, 255, 255, 255 };

// Blink red while immortal after a hit.
static void draw_player_hit(GameState* gs, SDL_Texture* t, int x, int y) {
	SDL_Rect dst = { x, y, 0, 0 };
	SDL_QueryTexture(t, NULL, NULL, &dst.w, &dst.h);
	dst.w *= 2;
	dst.h *= 2;
	int red = (int)((SDL_GetTicks64() / 100) % 2);
	if (red) SDL_SetTextureColorMod(t, 255, 0, 0);
	SDL_RenderCopyEx(gs->renderer, t, NULL, &dst, 0, NULL, gs->player.flip);
	if (red) SDL_SetTextureColorMod(t, 255, 255, 255);
}

static void render_character_animation(GameState* gs, SDL_Texture* texture[], const int width, const int height, int frame) {
	int x = (int)(gs->scrollX + gs->player.x), y = (int)gs->player.y;
	if (gs->player.isTakenDamage && gs->player.isImmortal) {
		draw_player_hit(gs, texture[frame], x, y);
	}
	else {
		SDL_Rect dstRect = { x, y, width * 2, height * 2 };
		SDL_RenderCopyEx(gs->renderer, texture[frame], NULL, &dstRect, 0, NULL, gs->player.flip);
	}
}

static void render_giga_golem(GameState* gs, SDL_Texture* texture[]) {
	int smoothness = animation_smoothness(10, GIGA_GOLEM_FRAMES);
	SDL_Rect srcRect = { 24 + smoothness * 100, 624, 50, 50 };
	Golem boss = gs->gigaGolem;
	SDL_Rect dstRect = { (int)(gs->scrollX + boss.pos.x), (int)boss.pos.y, WIDTH_GIGA_GOLEM, HEIGHT_GIGA_GOLEM };
	SDL_RenderCopyEx(gs->renderer, texture[0], &srcRect, &dstRect, 0, NULL, 0);
}

// Leak #1 fix: text texture is rebuilt only when the number changes.
static void hud_label(GameState* gs, SDL_Texture** tex, int* cached, int value) {
	if (*tex && *cached == value) return;
	res_destroy_texture(tex);
	char s[16];
	SDL_snprintf(s, sizeof s, "%d", value);
	*tex = res_text(gs->renderer, gs->hud_font, s, WHITE);
	*cached = value;
}

static void render_status(GameState* gs) {
	hud_label(gs, &gs->label_lives, &gs->cached_lives, gs->player.lives);
	hud_label(gs, &gs->label_health_potion, &gs->cached_potions, gs->health_potion_counter);
	SDL_Rect textRect1 = { 5, -15, 0, 0 };
	SDL_QueryTexture(gs->label_lives, NULL, NULL, &textRect1.w, &textRect1.h);
	SDL_Rect textRect2 = { 200, -15, 0, 0 };
	SDL_QueryTexture(gs->label_health_potion, NULL, NULL, &textRect2.w, &textRect2.h);
	int smoothness1 = animation_smoothness(6, FIRST_AID_KIT_FRAMES);
	SDL_Rect srcRect1 = { smoothness1 * 32, 1 * 32, 32, 32 };
	int smoothness2 = animation_smoothness(8, FIRST_AID_KIT_FRAMES);
	SDL_Rect srcRect2 = { smoothness2 * 32, 9 * 32, 32, 32 };
	SDL_RenderCopy(gs->renderer, gs->label_lives, NULL, &textRect1);
	SDL_RenderCopy(gs->renderer, gs->items[0], &srcRect1, &(SDL_Rect){ textRect1.x + textRect1.w, 0, 48, 48 });
	SDL_RenderCopy(gs->renderer, gs->label_health_potion, NULL, &textRect2);
	SDL_RenderCopy(gs->renderer, gs->items[0], &srcRect2, &(SDL_Rect){ textRect2.x + textRect2.w, 0, 48, 48 });
}

// Leak #1b fix: title/hint created once per ending (game_reset destroys them).
static void render_over(GameState* gs, const char* str, SDL_Color fg) {
	Mix_HaltChannel(-1);
	if (!Mix_PlayingMusic()) {
		Mix_PlayMusic(gs->soundTracks.ending[gs->statusState], -1); // statusState 0..2 == ending index
	}
	for (int i = 0; i < 5; i++) {
		SDL_RenderCopy(gs->renderer, gs->background[i].layer[0], NULL, NULL);
	}
	if (!gs->over_title) {
		gs->over_title = res_text(gs->renderer, gs->over_font, str, fg);
	}
	SDL_Rect t = { 0 };
	SDL_QueryTexture(gs->over_title, NULL, NULL, &t.w, &t.h);
	t.x = (WIDTH_WINDOW - t.w) / 2;
	t.y = (HEIGHT_WINDOW - t.h) / 2;
	SDL_RenderCopy(gs->renderer, gs->over_title, NULL, &t);
	if (SDL_GetTicks64() / 1000.0f - gs->overTiming >= 2.0f) {
		if (!gs->over_hint) {
			gs->over_hint = res_text(gs->renderer, gs->hud_font, "Press R to retry, X to return menu", (SDL_Color){ 255, 255, 73, 255 });
		}
		SDL_Rect h = { 0, -15, 0, 0 };
		SDL_QueryTexture(gs->over_hint, NULL, NULL, &h.w, &h.h);
		SDL_RenderCopy(gs->renderer, gs->over_hint, NULL, &h);
	}
}

void do_render(GameState* gs) {
	SDL_Renderer* renderer = gs->renderer;
	SDL_RenderClear(renderer);
	if (gs->statusState == STATUS_STATE_GAMEOVER) {
		render_over(gs, "Bad End", (SDL_Color){ 210, 8, 45, 255 });
	}
	else if (gs->statusState == STATUS_STATE_TRUE_END) {
		render_over(gs, "True End", (SDL_Color){ 200, 112, 126, 255 });
	}
	else if (gs->statusState == STATUS_STATE_GOOD_END) {
		render_over(gs, "Good End", (SDL_Color){ 96, 106, 116, 255 });
	}
	else if (gs->statusState == STATUS_STATE_GAME) {
		drawBackground(gs);
		for (int i = 0; i < gs->map->counter; i++) {
			if (gs->isKeyObtained && !gs->flag) {
				draw_key_map(gs, 23);
			}
			drawMap(gs, &gs->map[i]);
		}
		drawLedges(gs);
		if (gs->player.status == 0) { // idle
			render_character_animation(gs, gs->idle_anim, WIDTH_PLAYER_IDLE, HEIGHT_PLAYER_IDLE, animation_smoothness(12, CHARACTER_FRAMES));
		}
		else if (gs->player.status == 1) { // run
			render_character_animation(gs, gs->run_anim, WIDTH_PLAYER_RUN, HEIGHT_PLAYER_RUN, animation_smoothness(8, CHARACTER_FRAMES));
		}
		else if (gs->player.status == 2) { // jump
			render_character_animation(gs, gs->jump_anim, WIDTH_PLAYER_JUMP, HEIGHT_PLAYER_JUMP, 0);
		}
		else if (gs->player.status == 3) { // mid air
			render_character_animation(gs, gs->jump_anim, WIDTH_PLAYER_JUMP, HEIGHT_PLAYER_JUMP, 1);
		}
		else if (gs->player.status == 4) { // falling
			render_character_animation(gs, gs->jump_anim, WIDTH_PLAYER_JUMP, HEIGHT_PLAYER_JUMP, animation_smoothness(2, CHARACTER_FRAMES) + 2);
		}
		render_giga_golem(gs, gs->golem);
		render_status(gs);
		if (!Mix_PlayingMusic()) {
			Mix_PlayMusic(gs->soundTracks.inGame[0], -1);
		}
	}
	SDL_RenderPresent(renderer);
}
```

Compile → không error.

### Task 2.7: `src/scenes/scene.h` + `gameplay.c`

**Step 1: `src/scenes/scene.h`**
```c
#ifndef SCENE_H
#define SCENE_H
#include "core/app.h"

typedef enum {
	SCENE_QUIT, SCENE_MENU, SCENE_STORY, SCENE_HELP, SCENE_DONATE, SCENE_GAMEMODE, SCENE_INTRO, SCENE_GAMEPLAY
} SceneId;

// Each scene owns what it loads, frees it before returning, and returns the next scene.
// No scene calls another scene.
SceneId menu_run(App* app);
SceneId story_run(App* app);
SceneId help_run(App* app);
SceneId donate_run(App* app);
SceneId gamemode_run(App* app);
SceneId intro_run(App* app);
SceneId gameplay_run(App* app);

static inline int ui_hit(const SDL_Rect* r, int x, int y) {
	return x >= r->x && x <= r->x + r->w && y >= r->y && y <= r->y + r->h;
}

static inline void ui_size(SDL_Texture* t, SDL_Rect* r) {
	SDL_QueryTexture(t, NULL, NULL, &r->w, &r->h);
}
#endif
```

**Step 2: `src/scenes/gameplay.c`**
```c
#include "scenes/scene.h"
#include "game/game.h"
#include <math.h>

static int over_ready(const GameState* gs) {
	return gs->isOver && SDL_GetTicks64() / 1000.0f - gs->overTiming >= 2.0f;
}

// Was processEvents(): now also owns R/X on the ending screen (render used to poll too and lost keys).
static SceneId handle_input(GameState* gs) {
	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) {
		gs->player.status = 1;
		gs->player.flip = 1;
		gs->player.dx -= 0.3f;
		if (gs->player.dx < -2.5f) gs->player.dx = -2.5f;
	}
	else if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) {
		gs->player.status = 1;
		gs->player.flip = 0;
		gs->player.dx += 0.3f;
		if (gs->player.dx > 2.5f) gs->player.dx = 2.5f;
	}
	else if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S]) {
		gs->player.dy += 0.3f;
	}
	else {
		gs->player.status = 0;
		gs->player.flip = 0;
		gs->player.dx *= 0.8f;
		if (fabsf(gs->player.dx) < 0.1f) gs->player.dx = 0;
	}
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) return SCENE_QUIT;
		if (event.type != SDL_KEYDOWN) continue;
		switch (event.key.keysym.sym) {
		case SDLK_SPACE:
		case SDLK_UP:
		case SDLK_w:
			if (gs->player.onLedge) {
				gs->player.onLedge = 0;
				gs->player.dy = -9;
			}
			break;
		case SDLK_r:
			if (over_ready(gs)) {
				Mix_HaltMusic();
				game_reset(gs); // leak #2 fix: state only, no reload
			}
			break;
		case SDLK_x:
			if (over_ready(gs)) return SCENE_MENU;
			break;
		}
	}
	return SCENE_GAMEPLAY;
}

SceneId gameplay_run(App* app) {
	static GameState gs; // ~470KB: keep it off the stack
	SDL_memset(&gs, 0, sizeof gs);
	gs.renderer = app->renderer;
	gs.difficulty = app->difficulty;
	app_resize(app, WIDTH_WINDOW, HEIGHT_WINDOW);

	SceneId next = SCENE_GAMEPLAY;
	if (game_assets_load(&gs)) {
		game_reset(&gs);
	}
	else {
		app_fatal(app, "Missing game resources (see console)");
		next = SCENE_QUIT;
	}
	Uint64 last = SDL_GetTicks64();
	while (next == SCENE_GAMEPLAY) {
		Uint64 now = SDL_GetTicks64();
		gs.dt = (now - last) / 1000.0f;
		last = now;
		next = handle_input(&gs);
		if (next != SCENE_GAMEPLAY) break;
		collision_detect_floor(&gs);
		for (int i = 0; i < gs.map->counter; i++) {
			collision_detect_map(&gs, &gs.map[i]);
		}
		add_physics(&gs);
		do_render(&gs);
		app_tick(app);
	}
	Mix_HaltChannel(-1);
	Mix_HaltMusic();
	game_assets_free(&gs);
	return next;
}
```

**Step 3:** Compile `src\scenes\gameplay.c;src\game\physics.c;src\game\collision.c;src\game\map.c;src\game\assets.c;src\game\render.c` → không error.

### Task 2.8: `src/scenes/menu.c` (ghi đè; login/register bị bỏ)

```c
#include "scenes/scene.h"
#include "core/res.h"

SceneId menu_run(App* app) {
	static const char* labels[6] = { "Single Player", "Story", "Game Mode", "Help", "Donate", "Exit" };
	static const SDL_Point at[6] = { { 400, 170 }, { 440, 225 }, { 410, 280 }, { 450, 335 }, { 430, 390 }, { 450, 445 } };
	static const SceneId target[6] = { SCENE_INTRO, SCENE_STORY, SCENE_GAMEMODE, SCENE_HELP, SCENE_DONATE, SCENE_QUIT };
	SDL_Renderer* r = app->renderer;
	app_resize(app, MENU_W, MENU_H);
	if (!Mix_PlayingMusic()) {
		Mix_VolumeMusic(32);
		Mix_PlayMusic(app->menuMusic, -1);
	}

	SDL_Texture* bg = res_texture(r, "Resource\\Menu\\LoginImage\\ChooseBG.bmp", 0);
	TTF_Font* font = res_font("Resource\\Fonts\\crazy-pixel.ttf", 50);
	SDL_Texture* normal[6], * hot[6];
	SDL_Rect pos[6];
	for (int i = 0; i < 6; i++) {
		normal[i] = res_text(r, font, labels[i], (SDL_Color){ 0, 0, 0, 255 });
		hot[i] = res_text(r, font, labels[i], (SDL_Color){ 255, 0, 0, 255 });
		pos[i] = (SDL_Rect){ at[i].x, at[i].y, 0, 0 };
		ui_size(normal[i], &pos[i]);
	}
	res_close_font(&font);

	SceneId next = SCENE_MENU;
	if (!bg || !normal[0]) {
		app_fatal(app, "Missing menu resources (see console)");
		next = SCENE_QUIT;
	}
	int hover = -1;
	while (next == SCENE_MENU) {
		SDL_Event e;
		while (next == SCENE_MENU && SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				next = SCENE_QUIT;
			}
			else if (e.type == SDL_MOUSEMOTION) {
				hover = -1;
				for (int i = 0; i < 6; i++) {
					if (ui_hit(&pos[i], e.motion.x, e.motion.y)) hover = i;
				}
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN) {
				for (int i = 0; i < 6; i++) {
					if (ui_hit(&pos[i], e.button.x, e.button.y)) {
						Mix_PlayChannel(-1, app->click, 0);
						next = target[i];
					}
				}
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				next = SCENE_QUIT;
			}
		}
		SDL_RenderClear(r);
		SDL_RenderCopy(r, bg, NULL, NULL);
		for (int i = 0; i < 6; i++) {
			SDL_RenderCopy(r, i == hover ? hot[i] : normal[i], NULL, &pos[i]);
		}
		SDL_RenderPresent(r);
		app_tick(app);
	}
	if (next == SCENE_INTRO) {
		Mix_HaltMusic();
	}
	res_destroy_seq(normal, 6);
	res_destroy_seq(hot, 6);
	res_destroy_texture(&bg);
	return next;
}
```
Compile → không error, không warning.

### Task 2.9: `src/scenes/story.c` (mới; từ `storyDisplay` cũ)

```c
#include "scenes/scene.h"
#include "core/res.h"

SceneId story_run(App* app) {
	SDL_Renderer* r = app->renderer;
	const SDL_Rect rightRect = { 714, 481, 73, 55 }, leftRect = { 206, 480, 74, 55 };
	const SDL_Rect fixScreen = { -70, 0, 1110, 668 };
	SDL_Texture* page[2] = {
		res_texture(r, "Resource\\Menu\\Story\\story1.bmp", 0),
		res_texture(r, "Resource\\Menu\\Story\\story2.bmp", 0),
	};
	SDL_Texture* red = res_texture(r, "Resource\\Menu\\Story\\red.bmp", 0);
	SDL_Texture* green = res_texture(r, "Resource\\Menu\\Story\\green.bmp", 0);
	TTF_Font* font = res_font("Resource\\Fonts\\Kanit-Light.ttf", 24);
	SDL_Texture* back = res_text(r, font, "Return", (SDL_Color){ 0, 0, 0, 255 });
	SDL_Texture* backHot = res_text(r, font, "Return", (SDL_Color){ 0, 255, 255, 255 });
	res_close_font(&font);
	SDL_Rect returnRect = { 450, 503, 0, 0 };
	ui_size(back, &returnRect);

	SceneId next = SCENE_STORY;
	if (!page[0] || !page[1] || !red || !green || !back) {
		app_fatal(app, "Missing story resources (see console)");
		next = SCENE_QUIT;
	}
	int current = 0, hotRight = 0, hotLeft = 0, hotBack = 0;
	while (next == SCENE_STORY) {
		SDL_Event e;
		while (next == SCENE_STORY && SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				next = SCENE_QUIT;
			}
			else if (e.type == SDL_MOUSEMOTION) {
				hotRight = ui_hit(&rightRect, e.motion.x, e.motion.y);
				hotLeft = ui_hit(&leftRect, e.motion.x, e.motion.y);
				hotBack = ui_hit(&returnRect, e.motion.x, e.motion.y);
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN) {
				int x = e.button.x, y = e.button.y;
				if (ui_hit(&rightRect, x, y)) {
					Mix_PlayChannel(-1, app->click, 0);
					current = 1;
				}
				if (ui_hit(&leftRect, x, y)) {
					Mix_PlayChannel(-1, app->click, 0);
					current = 0;
				}
				if (ui_hit(&returnRect, x, y)) {
					Mix_PlayChannel(-1, app->click, 0);
					next = SCENE_MENU;
				}
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				next = SCENE_MENU;
			}
		}
		SDL_RenderClear(r);
		SDL_RenderCopy(r, page[current], NULL, &fixScreen);
		if (current == 0) { // page 1: "next" arrow on the right
			SDL_RenderCopyEx(r, hotRight ? red : green, NULL, &rightRect, 0, NULL, hotRight ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
		}
		else {              // page 2: "back" arrow on the left
			SDL_RenderCopyEx(r, hotLeft ? red : green, NULL, &leftRect, 0, NULL, hotLeft ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
		}
		SDL_RenderCopy(r, hotBack ? backHot : back, NULL, &returnRect);
		SDL_RenderPresent(r);
		app_tick(app);
	}
	res_destroy_seq(page, 2);
	res_destroy_texture(&red);
	res_destroy_texture(&green);
	res_destroy_texture(&back);
	res_destroy_texture(&backHot);
	return next;
}
```
Compile → không error, không warning.

### Task 2.10: `src/scenes/help.c` (ghi đè; bỏ toàn bộ biến global)

```c
#include "scenes/scene.h"
#include "core/res.h"

SceneId help_run(App* app) {
	SDL_Renderer* r = app->renderer;
	const SDL_Rect healthRect = { 550, 302, 40, 40 }, goodRect = { 460, 365, 40, 40 };
	const SDL_Rect runningRect = { 500, 130, 35, 65 }, jumpingRect = { 430, 214, 35, 65 };
	const SDL_Rect ceilingRect = { 610, 410, 60, 50 }, fireRect = { 670, 370, 55, 90 };
	const SDL_Rect sawRect = { 720, 410, 50, 50 }, groundRect = { 750, 410, 70, 50 };
	const SDL_Rect lightningRect = { 793, 400, 70, 60 };

	SDL_Texture* bg = res_texture(r, "Resource\\Menu\\LoginImage\\help.bmp", 0);
	SDL_Texture* fire = res_texture(r, "Resource\\Menu\\traps\\fire.bmp", 0);
	SDL_Texture* saw = res_texture(r, "Resource\\Menu\\traps\\saw.bmp", 0);
	SDL_Texture* ground = res_texture(r, "Resource\\Menu\\traps\\ground.bmp", 0);
	SDL_Texture* lightning = res_texture(r, "Resource\\Menu\\traps\\lightning.bmp", 0);
	SDL_Texture* ceiling = res_texture(r, "Resource\\Menu\\traps\\ceiling.bmp", 0);
	SDL_Texture* health[7], * good[8], * running[8], * jumping[5];
	res_texture_seq(r, health, 7, "Resource\\Menu\\item\\health%d.bmp", 1);
	res_texture_seq(r, good, 8, "Resource\\Menu\\item\\good%d.bmp", 1);
	res_texture_seq(r, running, 8, "Resource\\Menu\\running\\run%d.bmp", 0);
	res_texture_seq(r, jumping, 5, "Resource\\Menu\\jump\\jump%d.bmp", 0);
	TTF_Font* font = res_font("Resource\\Fonts\\Kanit-Light.ttf", 24);
	SDL_Texture* back = res_text(r, font, "Return", (SDL_Color){ 0, 0, 0, 255 });
	SDL_Texture* backHot = res_text(r, font, "Return", (SDL_Color){ 0, 255, 255, 255 });
	res_close_font(&font);
	SDL_Rect returnRect = { 446, 503, 0, 0 };
	ui_size(back, &returnRect);

	SceneId next = SCENE_HELP;
	if (!bg || !back) {
		app_fatal(app, "Missing help resources (see console)");
		next = SCENE_QUIT;
	}
	int hotBack = 0;
	while (next == SCENE_HELP) {
		SDL_Event e;
		while (next == SCENE_HELP && SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				next = SCENE_QUIT;
			}
			else if (e.type == SDL_MOUSEMOTION) {
				hotBack = ui_hit(&returnRect, e.motion.x, e.motion.y);
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN && ui_hit(&returnRect, e.button.x, e.button.y)) {
				Mix_PlayChannel(-1, app->click, 0);
				next = SCENE_MENU;
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				next = SCENE_MENU;
			}
		}
		Uint64 t = SDL_GetTicks64();
		SDL_RenderClear(r);
		SDL_RenderCopy(r, bg, NULL, NULL);
		SDL_RenderCopy(r, hotBack ? backHot : back, NULL, &returnRect);
		SDL_RenderCopy(r, lightning, NULL, &lightningRect);
		SDL_RenderCopy(r, ground, NULL, &groundRect);
		SDL_RenderCopy(r, fire, NULL, &fireRect);
		SDL_RenderCopy(r, saw, NULL, &sawRect);
		SDL_RenderCopy(r, health[t / 120 % 7], NULL, &healthRect);
		SDL_RenderCopy(r, good[t / 120 % 8], NULL, &goodRect);
		SDL_RenderCopy(r, running[t / 120 % 8], NULL, &runningRect);
		SDL_RenderCopy(r, jumping[t / 200 % 5], NULL, &jumpingRect);
		SDL_RenderCopy(r, ceiling, NULL, &ceilingRect);
		SDL_RenderPresent(r);
		app_tick(app);
	}
	res_destroy_texture(&bg);
	res_destroy_texture(&fire);
	res_destroy_texture(&saw);
	res_destroy_texture(&ground);
	res_destroy_texture(&lightning);
	res_destroy_texture(&ceiling);
	res_destroy_seq(health, 7);
	res_destroy_seq(good, 8);
	res_destroy_seq(running, 8);
	res_destroy_seq(jumping, 5);
	res_destroy_texture(&back);
	res_destroy_texture(&backHot);
	return next;
}
```
Compile → không error, không warning.

### Task 2.11: `src/scenes/donate.c` (mới; từ `QRcode` cũ)

```c
#include "scenes/scene.h"
#include "core/res.h"

SceneId donate_run(App* app) {
	SDL_Renderer* r = app->renderer;
	const SDL_Rect qrRect = { 398, 255, 170, 170 }, logoRect = { 328, 300, 60, 60 };
	SDL_Texture* bg = res_texture(r, "Resource\\Menu\\LoginImage\\donate.bmp", 0);
	SDL_Texture* qr = res_texture(r, "Resource\\Menu\\LoginImage\\QR.bmp", 0);
	SDL_Texture* logo = res_texture(r, "Resource\\Menu\\LoginImage\\momologo.bmp", 0);
	TTF_Font* font = res_font("Resource\\Fonts\\Kanit-Light.ttf", 24);
	SDL_Texture* back = res_text(r, font, "Return", (SDL_Color){ 0, 0, 0, 255 });
	SDL_Texture* backHot = res_text(r, font, "Return", (SDL_Color){ 0, 255, 255, 255 });
	res_close_font(&font);
	font = res_font("Resource\\Fonts\\Kanit-MediumItalic.ttf", 30);
	SDL_Texture* thanks = res_text(r, font, "^^Thank You For Your Donation^^", (SDL_Color){ 0, 255, 0, 255 });
	res_close_font(&font);
	SDL_Rect returnRect = { 447, 440, 0, 0 }, thankRect = { 247, 200, 0, 0 };
	ui_size(back, &returnRect);
	ui_size(thanks, &thankRect);

	SceneId next = SCENE_DONATE;
	if (!bg || !back) {
		app_fatal(app, "Missing donate resources (see console)");
		next = SCENE_QUIT;
	}
	int hotBack = 0;
	while (next == SCENE_DONATE) {
		SDL_Event e;
		while (next == SCENE_DONATE && SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				next = SCENE_QUIT;
			}
			else if (e.type == SDL_MOUSEMOTION) {
				hotBack = ui_hit(&returnRect, e.motion.x, e.motion.y);
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN && ui_hit(&returnRect, e.button.x, e.button.y)) {
				Mix_PlayChannel(-1, app->click, 0);
				next = SCENE_MENU;
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				next = SCENE_MENU;
			}
		}
		SDL_RenderClear(r);
		SDL_RenderCopy(r, bg, NULL, NULL);
		SDL_RenderCopy(r, qr, NULL, &qrRect);
		SDL_RenderCopy(r, logo, NULL, &logoRect);
		SDL_RenderCopy(r, thanks, NULL, &thankRect);
		SDL_RenderCopy(r, hotBack ? backHot : back, NULL, &returnRect);
		SDL_RenderPresent(r);
		app_tick(app);
	}
	res_destroy_texture(&bg);
	res_destroy_texture(&qr);
	res_destroy_texture(&logo);
	res_destroy_texture(&thanks);
	res_destroy_texture(&back);
	res_destroy_texture(&backHot);
	return next;
}
```
Compile → không error, không warning.

### Task 2.12: `src/scenes/gamemode.c` (mới; từ `gameMode` cũ)

```c
#include "scenes/scene.h"
#include "core/res.h"
#include "game/defs.h"

SceneId gamemode_run(App* app) {
	static const char* modes[3] = { "Easy", "Medium", "Hard" };
	static const int difficulty[3] = { DIFFICULTY_EASY, DIFFICULTY_MEDIUM, DIFFICULTY_HARD };
	static const SDL_Color modeColor[3] = { { 0, 255, 0, 255 }, { 255, 200, 0, 255 }, { 255, 0, 0, 255 } };
	static const int modeX[3] = { 435, 415, 435 };
	const SDL_Color black = { 0, 0, 0, 255 }, red = { 255, 0, 0, 255 };
	const SDL_Rect trickRect = { 390, 300, 190, 70 };
	SDL_Renderer* r = app->renderer;

	SDL_Texture* bg = res_texture(r, "Resource\\Menu\\LoginImage\\gamemode.bmp", 0);
	SDL_Texture* trick = res_texture(r, "Resource\\Menu\\LoginImage\\trick3.bmp", 0);
	TTF_Font* font = res_font("Resource\\Fonts\\Kanit-Light.ttf", 24);
	SDL_Texture* back = res_text(r, font, "Return", black);
	SDL_Texture* backHot = res_text(r, font, "Return", (SDL_Color){ 0, 255, 255, 255 });
	res_close_font(&font);
	font = res_font("Resource\\Fonts\\Kanit-Medium.ttf", 60);
	SDL_Texture* arrow[2] = { res_text(r, font, "<<", black), res_text(r, font, ">>", black) };
	SDL_Texture* arrowHot[2] = { res_text(r, font, "<<", red), res_text(r, font, ">>", red) };
	res_close_font(&font);
	font = res_font("Resource\\Fonts\\crazy-pixel.ttf", 100);
	SDL_Texture* title = res_text(r, font, "Game Mode", (SDL_Color){ 0, 200, 255, 255 });
	res_close_font(&font);
	TTF_Font* modeFont = res_font("Resource\\Fonts\\crazy-pixel.ttf", 80);
	SDL_Texture* mode = res_text(r, modeFont, modes[app->modeIndex], modeColor[app->modeIndex]);

	SDL_Rect returnRect = { 446, 439, 0, 0 }, titleRect = { 350, 130, 0, 0 };
	SDL_Rect arrowRect[2] = { { 310, 290, 0, 0 }, { 595, 290, 0, 0 } };
	ui_size(back, &returnRect);
	ui_size(title, &titleRect);
	ui_size(arrow[0], &arrowRect[0]);
	ui_size(arrow[1], &arrowRect[1]);

	SceneId next = SCENE_GAMEMODE;
	if (!bg || !back || !modeFont) {
		app_fatal(app, "Missing game mode resources (see console)");
		next = SCENE_QUIT;
	}
	int hotBack = 0, hotArrow[2] = { 0, 0 };
	while (next == SCENE_GAMEMODE) {
		SDL_Event e;
		while (next == SCENE_GAMEMODE && SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				next = SCENE_QUIT;
			}
			else if (e.type == SDL_MOUSEMOTION) {
				hotBack = ui_hit(&returnRect, e.motion.x, e.motion.y);
				for (int i = 0; i < 2; i++) hotArrow[i] = ui_hit(&arrowRect[i], e.motion.x, e.motion.y);
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN) {
				int x = e.button.x, y = e.button.y;
				if (ui_hit(&returnRect, x, y)) {
					Mix_PlayChannel(-1, app->click, 0);
					next = SCENE_MENU;
				}
				int step = ui_hit(&arrowRect[0], x, y) ? 2 : ui_hit(&arrowRect[1], x, y) ? 1 : 0; // 2 == -1 mod 3
				if (step) {
					app->modeIndex = (app->modeIndex + step) % 3;
					app->difficulty = difficulty[app->modeIndex];
					res_destroy_texture(&mode);
					mode = res_text(r, modeFont, modes[app->modeIndex], modeColor[app->modeIndex]);
				}
			}
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				next = SCENE_MENU;
			}
		}
		SDL_Rect modeRect = { modeX[app->modeIndex], 265, 0, 0 };
		ui_size(mode, &modeRect);
		SDL_RenderClear(r);
		SDL_RenderCopy(r, bg, NULL, NULL);
		SDL_RenderCopy(r, trick, NULL, &trickRect);
		SDL_RenderCopy(r, hotBack ? backHot : back, NULL, &returnRect);
		for (int i = 0; i < 2; i++) {
			SDL_RenderCopy(r, hotArrow[i] ? arrowHot[i] : arrow[i], NULL, &arrowRect[i]);
		}
		SDL_RenderCopy(r, title, NULL, &titleRect);
		SDL_RenderCopy(r, mode, NULL, &modeRect);
		SDL_RenderPresent(r);
		app_tick(app);
	}
	res_destroy_texture(&bg);
	res_destroy_texture(&trick);
	res_destroy_texture(&back);
	res_destroy_texture(&backHot);
	res_destroy_seq(arrow, 2);
	res_destroy_seq(arrowHot, 2);
	res_destroy_texture(&title);
	res_destroy_texture(&mode);
	res_close_font(&modeFont);
	return next;
}
```
Bỏ hành vi cũ “vào Game Mode là reset về Easy”. Compile → không error, không warning.

### Task 2.13: `src/scenes/intro.c` (ghi đè; từ `PreGameplay` cũ)

```c
#include "scenes/scene.h"
#include "core/res.h"
#include "game/defs.h"
#include <string.h>

static const char* story[7] = {
	"\"...In 2016, Chris, who wanted to cure his beloved sister Claire's late-stage cancer, set out for the",
	"forest after obtaining the location from a lucky explorer who survived the forest. He hoped to find the",
	"treasure in the legend and use its power to save his sister. He crossed through dangerous creatures and",
	"reached the relic. He advanced deep into the relic, trying to hide and find a way to defeat the Golem.",
	"finally. The Golem chased after him and used its power to trap him in an endless loop of being pursued",
	"until death. Chris, with no weapons except for the sword he picked up from the relic, had to break the",
	"loop and defeat the Golem to reach the treasure.\""
};

// Typewriter intro. Draws incrementally on purpose: earlier lines stay on screen,
// each step only repaints from the current line down (same as the original).
SceneId intro_run(App* app) {
	SDL_Renderer* r = app->renderer;
	const SDL_Color white = { 255, 255, 255, 255 };
	app_resize(app, WIDTH_WINDOW, HEIGHT_WINDOW);

	Mix_Music* typing = res_mus("Resource\\Sound\\Sound Effects\\typing.mp3");
	Mix_PlayMusic(typing, -1);
	Mix_VolumeMusic(64);
	TTF_Font* font = res_font("Resource\\Fonts\\crazy-pixel.ttf", 80);
	SDL_Texture* skip = res_text(r, font, ">> Press [SPACE] to skip <<", white);
	res_close_font(&font);
	font = res_font("Resource\\Fonts\\Kanit-Light.ttf", 20);
	SDL_Texture* black = res_texture(r, "Resource\\Menu\\LoginImage\\bg.bmp", 0);
	SDL_Texture* whitebar = res_texture(r, "Resource\\Menu\\LoginImage\\whitebar.bmp", 0);
	SDL_Texture* blackbar = res_texture(r, "Resource\\Menu\\LoginImage\\blackbar.bmp", 0);
	SDL_Texture* line = NULL;

	SDL_Rect skipRect = { 290, 500, 0, 0 };
	ui_size(skip, &skipRect);
	SDL_Rect lineRect = { 120, 76, 0, 0 };
	SDL_Rect underline = { 100, 100, 15, 5 };
	SDL_Rect blackRect = { 0, 100, 1200, 800 };
	int charNumber = 0, lineNumber = 0;

	SceneId next = SCENE_INTRO;
	if (!font || !black || !whitebar || !blackbar) {
		app_fatal(app, "Missing intro resources (see console)");
		next = SCENE_QUIT;
	}
	SDL_RenderClear(r);
	while (next == SCENE_INTRO) {
		SDL_Event e;
		while (next == SCENE_INTRO && SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) next = SCENE_QUIT;
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) next = SCENE_MENU;
			else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) next = SCENE_GAMEPLAY;
		}
		if (next != SCENE_INTRO) break;
		if (charNumber >= (int)strlen(story[lineNumber])) {
			lineNumber++;
			if (lineNumber >= 7) { // was an out-of-bounds read of story[7]
				next = SCENE_GAMEPLAY;
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
		SDL_strlcpy(prefix, story[lineNumber], SDL_min(sizeof prefix, (size_t)charNumber + 1));
		res_destroy_texture(&line); // leak #5 fix (and old lineTexture[2] overflow)
		line = res_text(r, font, prefix, white);
		ui_size(line, &lineRect);
		underline.x += 44;

		SDL_Delay(100);
		SDL_RenderCopy(r, black, NULL, &blackRect);
		SDL_RenderCopy(r, line, NULL, &lineRect);
		SDL_RenderCopy(r, skip, NULL, &skipRect);
		SDL_RenderPresent(r);
		SDL_Delay(60);
		SDL_RenderCopy(r, underline.x >= 1000 ? blackbar : whitebar, NULL, &underline);
		SDL_RenderPresent(r);
		app_tick(app);
	}
	Mix_HaltMusic();
	res_free_mus(&typing);
	res_close_font(&font);
	res_destroy_texture(&skip);
	res_destroy_texture(&black);
	res_destroy_texture(&whitebar);
	res_destroy_texture(&blackbar);
	res_destroy_texture(&line);
	return next;
}
```
Compile → không error, không warning.

### Task 2.14: `src/main.c` (ghi đè)

```c
#include "core/app.h"
#include "core/res.h"
#include "scenes/scene.h"

#ifdef _DEBUG
static const char* scene_name(SceneId s) {
	static const char* names[] = { "QUIT", "MENU", "STORY", "HELP", "DONATE", "GAMEMODE", "INTRO", "GAMEPLAY" };
	return names[s];
}
#endif

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;
	App app;
	if (!app_init(&app)) {
		app_quit(&app);
		return 1;
	}
	SceneId scene = SCENE_MENU;
	while (scene != SCENE_QUIT) {
#ifdef _DEBUG
		SceneId prev = scene;
#endif
		switch (scene) {
		case SCENE_MENU:     scene = menu_run(&app);     break;
		case SCENE_STORY:    scene = story_run(&app);    break;
		case SCENE_HELP:     scene = help_run(&app);     break;
		case SCENE_DONATE:   scene = donate_run(&app);   break;
		case SCENE_GAMEMODE: scene = gamemode_run(&app); break;
		case SCENE_INTRO:    scene = intro_run(&app);    break;
		case SCENE_GAMEPLAY: scene = gameplay_run(&app); break;
		default:             scene = SCENE_QUIT;         break;
		}
#ifdef _DEBUG
		char counts[128];
		res_counts(counts, sizeof counts);
		SDL_Log("[scene %s -> %s] %s", scene_name(prev), scene_name(scene), counts);
#endif
	}
	app_quit(&app);
	return 0;
}
```

### Task 2.15: Build cả project + commit GĐ2

**Step 1: Debug**
```bash
"$MSB" "Jungle Run.vcxproj" -t:Rebuild -p:Configuration=Debug -p:Platform=x64 -p:PlatformToolset=v145 -nologo -v:minimal 2>&1 | tee /tmp/jr-debug.log | grep -E " error |Jungle Run.exe"
grep -oE "warning C[0-9]+" /tmp/jr-debug.log | sort | uniq -c
grep -E "src\\\\(core|scenes)\\\\.* warning" /tmp/jr-debug.log   # Expected: không có
```
Expected: `Jungle Run.vcxproj -> ...\Jungle Run.exe`, 0 error, tổng warning < 98.

**Step 2: Release** — cùng lệnh với `-p:Configuration=Release`. Expected: build thành công.

**Step 3: Commit**
```bash
git add -A src "Jungle Run.vcxproj" "Jungle Run.vcxproj.filters"
git status --short   # chỉ src/, vcxproj, filters, và các file đã xóa
git commit -m "refactor: single window scene loop, resources owned per scene, fix memory leaks

- scenes return next SceneId instead of calling each other (no recursion, one window/renderer)
- gameplay loads assets once, retry only resets state
- HUD/ending text cached instead of TTF_OpenFont + texture every frame
- menu hover uses prebuilt textures; login/register removed
- Debug build shows live texture/font/sound counts in window title"
```

---

## GĐ3 — Rà soát & tài liệu

### Task 3.1: Audit tạo/hủy tài nguyên

```bash
# 1. Mọi tạo tài nguyên đi qua res.c
grep -rnE "SDL_CreateTexture|TTF_OpenFont|TTF_Render|Mix_Load|IMG_Load|SDL_LoadBMP" src | grep -v "^src/core/res.c"
# Expected: không có output

# 2. Chỉ app.c khởi tạo SDL / window / renderer / audio
grep -rnE "SDL_Init|SDL_CreateWindow|SDL_CreateRenderer|Mix_OpenAudio|TTF_Init|IMG_Init" src | grep -v "^src/core/app.c"
# Expected: không có output

# 3. Không scene nào gọi scene khác
grep -rnE "\b(menu|story|help|donate|gamemode|intro|gameplay)_run\(" src/scenes | grep -v "SceneId [a-z]*_run(App\* app) {"
# Expected: không có output

# 4. Không còn API cũ
grep -rnE "accountDisplay|loginDisplay|registerDisplay|PreGameplay|gamePlay\(|load_game\(|checkAccount|user\.txt" src
# Expected: không có output
```

Sau đó đọc từng scene: mỗi `res_texture/res_text/res_font/res_wav/res_mus` phải có `res_destroy_*/res_close_font/res_free_*` tương ứng trước `return next;` (hoặc nằm trong `game_assets_free`). Font nào chỉ dùng để tạo text tĩnh thì đã đóng ngay sau khi tạo.

### Task 3.2: README

**Files:** Modify `README.md` — thêm mục ngắn:
- Cấu trúc `src/core`, `src/scenes`, `src/game`.
- Build: mở `Jungle Run.sln`, chọn `x64`, Debug hoặc Release. Nếu máy chỉ có toolset mới hơn v143: *Project → Retarget Solution* hoặc build dòng lệnh với `-p:PlatformToolset=v145`.
- Bản Debug: tiêu đề cửa sổ hiển thị `tex/font/wav/mus/sdl_alloc`; console in một dòng mỗi lần đổi màn. Con số ở menu phải trở lại như cũ sau khi ra/vào các màn; nếu tăng dần là còn leak.

### Task 3.3: Cập nhật trạng thái thiết kế + commit

`docs/plans/2026-10-23-refactor-memory-leak-design.md`: dòng trạng thái → `đã triển khai, chờ kiểm thử runtime`; thêm vào nhật ký quyết định các mục 1–7 của phần “Sai khác so với thiết kế” ở trên (nếu được duyệt).

```bash
git add README.md docs/plans
git commit -m "docs: build notes and debug counter usage"
```

### Task 3.4: Bàn giao kiểm thử runtime

Gửi người dùng checklist mục 6 của thiết kế. Đặc biệt nhờ chú ý:
- Vị trí/hover của chữ ở menu, story, help, donate, game mode so với bản cũ (đổi sang vẽ lại toàn bộ mỗi frame).
- Intro chữ chạy vẫn giữ các dòng trước.
- Độ khó chọn ở Game Mode có tác dụng khi vào chơi (số máu 5/3/1).
- R/X ở màn kết thúc phản hồi ngay mỗi lần bấm.
