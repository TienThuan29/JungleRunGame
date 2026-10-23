# Jungle-Run-F-code
[![Build](https://github.com/TienThuan29/JungleRunGame/actions/workflows/release.yml/badge.svg)](https://github.com/TienThuan29/JungleRunGame/actions/workflows/release.yml)

**[⬇ Download Jungle Run for Windows (x64)](https://github.com/TienThuan29/JungleRunGame/releases/latest/download/JungleRun.zip)** — unzip, then run `Jungle Run.exe`. Every merge to `main` publishes a new build ([all releases](https://github.com/TienThuan29/JungleRunGame/releases)).

Welcome to Project "Jungle Run".
Q&A:
1/. How to set up SDL library to Microsoft Visual Studio?
https://thenumb.at/cpp-course/sdl2/01/vsSetup.html
You can see the link above but we have customized things differently. Let me guide you into the things we change:
  1. Platform: x64
  2. Change x86 -> x64.
  3. In Property Pages -> Linker -> Syste -> SubSystem -> chose Windows (/SUBSYSTEM:WINDOWS).

2/. Build
- Open `Jungle Run.sln`, pick `x64` and `Debug` or `Release`. SDL include/lib paths are already set for both.
- If your Visual Studio only has a newer toolset than v143: *Project -> Retarget Solution*, or build from the command line:
  `MSBuild "Jungle Run.vcxproj" -p:Configuration=Debug -p:Platform=x64 -p:PlatformToolset=v145`
- Run from the repo root (resources are loaded from `Resource\...`).
- The project is C++17 and builds at warning level 4 with no warnings; keep it that way.

3/. Source layout (C++, namespace `jr`)
- `src/main.cpp` — creates `jr::Game` and runs it.
- `src/core/` — `Game` (the one window/renderer/audio device; creates and destroys one scene at a time), `SdlHandles` (RAII `unique_ptr` wrappers for every texture/font/sound + debug counters).
- `src/ui/` — `Button` (text that changes color on hover).
- `src/scenes/` — `Scene` base class + Menu, Story, Help, Donate, GameMode, Intro, Gameplay. A scene owns what it loads; it is all released when the scene is destroyed.
- `src/game/` — gameplay: `World` (rules of one run), `Level` (ground + map segments), `Player`, `GigaGolem`, `Parallax`, `Hud`, `LevelLoader`, `GameAssets`, and the level entities `Tile`, `Trap`, `Pickup`, `SmolGolem`, `Decoration` (created by `EntityFactory` from tile codes).
- No `new`/`delete`, no static RAII objects, no exceptions for control flow.

4/. Level format (`Resource/Levels`)
- `course.txt` lists the run from left to right: `ground N` (N platforms, 336 px each), `map lvA [lvB ...]` (one 1200 px screen per grid file), `onkey lvA lvB` (when the key is picked up, screen lvA is rebuilt from grid lvB), `finish X` (pixels from the left edge where the run ends; must not exceed the course length). `#` starts a comment.
- `lvNN.txt` is a grid of 16 rows x 24 numbers (row 0 = top, 50 px cells). The tile codes are listed in the comment block at the top of every grid file.
- Edit the files and restart the game; no rebuild needed. A broken file shows a message box like `Resource\Levels\lv07.txt line 12: expected 24 numbers, found 23` after the intro, then returns to the menu.

5/. Adding a trap
1. Put the image in `Resource/` and add a `TextureId` + load line in `src/game/GameAssets.*`.
2. Same behaviour as an existing trap (animation + damage on some frames): add one `{ code, TrapSpec{...} }` row in `src/game/EntityFactory.cpp`. New behaviour: add a subclass of `Entity` and a `case code:` in `makeEntity`.
3. Raise `kMaxTileCode` in `src/game/Config.h`.
4. Use the code in a `lvNN.txt` and add it to the legend comment.

6/. Checking for memory leaks (Debug build)
- The window title shows `tex / font / wav / mus / sdl_alloc` (live counts), updated every second.
- The console prints one line per scene change with the same counts.
- Baseline = the numbers the first time you are on the menu in that build. Back on the menu they should return to the baseline; if they keep growing, something is leaking.
