#pragma once
#include <SDL.h>

namespace jr::cfg {
constexpr int kWindowW = 1200, kWindowH = 800;  // gameplay / intro
constexpr int kMenuW = 960, kMenuH = 668;       // menu scenes

constexpr int kDifficultyEasy = 5, kDifficultyMedium = 3, kDifficultyHard = 1;  // also max lives
constexpr int kLives[3] = { kDifficultyEasy, kDifficultyMedium, kDifficultyHard };  // by Game Mode index
constexpr int kRequiredPotions = 5;
constexpr int kMaxWorlds = 3;

constexpr float kStep = 1.0f / 60.0f;  // gameplay tick; per-tick constants (run, jump, gravity) assume it
constexpr int kMaxTicksPerFrame = 5;

constexpr int kTile = 50;
constexpr int kGridRows = 16, kGridCols = 24;
constexpr int kSegmentW = kTile * kGridCols;  // 1200
constexpr int kGroundW = 336, kGroundH = 159; // "Platform 3"
constexpr int kGroundTop = kWindowH - kGroundH;  // 641
constexpr int kMaxTileCode = 26;

constexpr int kCharacterFrames = 90;  // ms per player animation frame
constexpr int kIdleW = 19, kIdleH = 32, kRunW = 21, kRunH = 33, kJumpW = 20, kJumpH = 35;
constexpr int kItemFrames = 100;      // ms per item animation frame
constexpr int kSmolGolemFrames = 100;
constexpr int kGigaGolemFrames = 100, kGigaGolemSize = 250;
constexpr int kParallaxSpeed[5] = { 1, 6, 5, 4, 3 };
}

namespace jr {
// Decorative animation only (global wall clock). Gameplay timing uses World::time().
inline int animFrame(int frames, int periodMs) {
	return static_cast<int>(SDL_GetTicks64() / static_cast<Uint64>(periodMs) % static_cast<Uint64>(frames));
}
}
