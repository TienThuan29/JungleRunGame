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
