# Jungle-Run-F-code
[![Build](https://github.com/TienThuan29/JungleRunGame/actions/workflows/release.yml/badge.svg)](https://github.com/TienThuan29/JungleRunGame/actions/workflows/release.yml)

**[⬇ Download Jungle Run for Windows (x64)](https://github.com/TienThuan29/JungleRunGame/releases/latest/download/JungleRun.zip)** — unzip, then run `Jungle Run.exe`. Every merge to `main` publishes a new build ([all releases](https://github.com/TienThuan29/JungleRunGame/releases)).

## How to play
Run right through three jungle worlds while a giant golem chases you. Collect 5 potions for the Good End.
- Move: arrows or A / D. Jump: Space, W or Up. Sword: J, K, F or Left Ctrl (cuts small golems, arrows and the boss's arm).
- Esc: pause (R restart, X menu). F11: fullscreen. After a run: R retry, X menu, N next world.
- Gamepad (Xbox / PlayStation): stick or D-pad move, A jump, X sword, Y retry, Start pause, B back, RB next world.
- Green star = checkpoint: on Easy and Medium you respawn there with full lives. Hard has no checkpoints and 1 life.
- Worlds: 1 The Relic, 2 Temple Ruins, 3 Golem's Lair (ends in a boss fight: stay on the ground when it fires its laser, jump over the arm it rolls, hit it while it is open). Finishing a world unlocks the next; best times are kept per world and difficulty.
- Settings (music, sounds, fullscreen), unlocked worlds and best times are saved in `save.txt` next to `Jungle Run.exe`. Delete it to start over.

## Tuning the game feel
`Resource/tuning.txt` holds every game-feel number (jump, gravity, run speed, sword reach, golem speed, boss fight timings, ...) with its unit. Edit it, save, and press R in game: the new values apply to the next run. A bad value shows a warning box and keeps the default. Set `debug 1` to enable test keys in the Release build: F1 invulnerable, F2 jump to the next checkpoint, F3 jump to the end of the world, F5 reload the file.

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
- The project is C++17 and builds at warning level 4 with no warnings; keep it that way (CI builds Debug and Release with warnings as errors).
- `tools\selfcheck.bat` builds a small console program that loads `worlds.txt`, every course and `tuning.txt` with the game's loaders and checks them (5 potions reachable, checkpoints, arena, tuning ranges). It opens no game window; CI runs it on every build.

3/. Source layout (C++, namespace `jr`)
- `src/main.cpp` — creates `jr::Game` and runs it.
- `src/core/` — `Game` (the one window/renderer/audio device/gamepad; creates and destroys one scene at a time; F11 and gamepad buttons are handled here for every scene), `SdlHandles` (RAII `unique_ptr` wrappers for every texture/font/sound + debug counters), `SaveFile` (`save.txt`).
- `src/ui/` — `Button` (text that changes color on hover) and `ButtonList` (mouse + keyboard menu list).
- `src/scenes/` — `Scene` base class + Menu, Story, Help, Donate, GameMode, Settings, WorldSelect, Intro, Gameplay. A scene owns what it loads; it is all released when the scene is destroyed.
- `src/game/` — gameplay: `World` (rules of one run: fixed 60 Hz ticks, checkpoints, projectiles, boss arena, results), `Level` (ground + map segments), `Player` (movement + sword), `GigaGolem` (chase + boss fight state machine), `Parallax`, `Particles`, `Hud`, `LevelLoader` (courses + `worlds.txt`), `Tuning` (`tuning.txt`), `GameAssets`, and the level entities `Tile`, `Trap`, `ArrowTrap`, `Pickup` (also the checkpoint star), `SmolGolem`, `Decoration` (created by `EntityFactory` from tile codes).
- Gameplay time is `World::time()` (ticks / 60), so pausing stops every timer; `animFrame()` (wall clock) is only for decorative animation.
- No `new`/`delete`, no static RAII objects, no exceptions for control flow.

4/. Level format (`Resource/Levels`)
- `worlds.txt` lists up to 3 worlds in play order: `<course file> <in-game music 1-3> <title>`.
- A course file (`course.txt`, `course2.txt`, ...) lists the run from left to right: `ground N` (N platforms, 336 px each), `map lvA [lvB ...]` (one 1200 px screen per grid file), `onkey lvA lvB` (when the key is picked up, screen lvA is rebuilt from grid lvB), and then exactly one of `finish X` (pixels from the left edge where the run ends; must not exceed the course length) or `arena` (the boss fight starts here; needs at least 4 ground platforms after it and no checkpoint past it). `#` starts a comment.
- Tile codes 23-26: checkpoint star, arrow trap shooting left / right, fast small golem. Put a checkpoint or an arrow trap on top of a solid tile.
- `lvNN.txt` is a grid of 16 rows x 24 numbers (row 0 = top, 50 px cells). The tile codes are listed in the comment block at the top of every grid file.
- Edit the files and restart the game; no rebuild needed. A broken file shows a message box like `Resource\Levels\lv07.txt line 12: expected 24 numbers, found 23` when the world starts, then returns to the menu. Run `tools\selfcheck.bat` to check all worlds at once.

5/. Adding a trap
1. Put the image in `Resource/` and add a `TextureId` + load line in `src/game/GameAssets.*`.
2. Same behaviour as an existing trap (animation + damage on some frames): add one `{ code, TrapSpec{...} }` row in `src/game/EntityFactory.cpp`. New behaviour: add a subclass of `Entity` and a `case code:` in `makeEntity`.
3. Raise `kMaxTileCode` in `src/game/Config.h`.
4. Use the code in a `lvNN.txt` and add it to the legend comment.

6/. Checking for memory leaks (Debug build)
- The window title shows `tex / font / wav / mus / sdl_alloc` (live counts), updated every second.
- The console prints one line per scene change with the same counts.
- Baseline = the numbers the first time you are on the menu in that build. Back on the menu they should return to the baseline; if they keep growing, something is leaking.

7/. Credits
- Character, jungle tileset and parallax background: "Awesome Jungle Pack" by Jesse Munguia (itch.io; free for personal and commercial use, the pack itself may not be redistributed).
- Golem, laser and arm sprites: "Mecha-stone Golem 0.1" (no license file in the pack).
- Traps: "Pixel Trap Pack" by Baldur, distributed by Foozle (CC0) — see `Resource/Foozle_2DTR0001_Pixel_Trap_Pack/Readme.txt`.
- Items: "platformer items" by Dani Maccari (free with attribution; no repackaging) — see `Resource/platformer items/README.txt`.
- Sparkle: from "Plee the Bear" by Julien Jorge (CC-BY-SA 3.0) — see `Resource/sparkle/README`.
- Sword slash effect: "Pixel art sword slash effect" by tbbk, OpenGameArt (CC0) — `Resource/slash-fx/LICENSE.txt`.
- Sword swing sound: "Swishes Sound Pack" by artisticdude, OpenGameArt (CC0) — `Resource/Sound/Sound Effects/LICENSE-swishes.txt`.
