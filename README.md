# Jungle-Run-F-code
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

3/. Source layout
- `src/main.c` — scene loop: each scene returns the next one.
- `src/core/` — `app` (the one window/renderer/audio device), `res` (every texture/font/sound is created and freed here).
- `src/scenes/` — menu, story, help, donate, gamemode, intro, gameplay. Each scene frees what it loaded before returning.
- `src/game/` — gameplay: assets, physics, collision, map, levels, render.

4/. Checking for memory leaks (Debug build)
- The window title shows `tex / font / wav / mus / sdl_alloc` (live counts), updated every second.
- The console prints one line per scene change with the same counts.
- Back on the menu, the numbers should return to what they were the first time. If they keep growing, something is leaking.
