#pragma once
#include <string>

namespace jr {
// Game-feel numbers, read from Resource\tuning.txt. Defaults = the values the game shipped with.
// Per-tick values assume 60 ticks per second.
struct Tuning {
	float runAccel = 0.3f, runMax = 2.5f, friction = 0.8f, jumpVy = -9.0f, gravity = 0.25f, fallAccel = 0.3f;
	float immortalSec = 1.0f, respawnImmortalSec = 2.0f;
	float attackSec = 0.2f, attackCooldownSec = 0.4f, attackW = 70.0f, attackH = 60.0f;
	float bossSpeed = 75.0f, bossDelaySec = 0.5f, bossFireSec = 4.0f, bossFireRange = 900.0f, bossWarnPx = 400.0f;
	float respawnBossGap = 900.0f;
	float armSpeed = 260.0f, arrowSpeed = 320.0f, arrowEverySec = 2.5f;
	float smolSpeed = 40.0f, smolMax = 120.0f, smolDetect = 150.0f, smolFastMul = 2.0f;
	float bossHp = 5.0f, bossIdleSec = 1.0f, bossChargeSec = 0.8f, bossBeamSec = 0.6f, bossOpenSec = 2.0f;
	float shakeHit = 8.0f, shakeDecay = 30.0f;
	float debug = 0.0f;  // 1 = F1-F3/F5 test keys in Release too
};

// Resets `t` to defaults, then applies `key value` lines from `path` (# starts a comment).
// A missing file is fine. Unknown keys and bad or out-of-range values keep the default and
// are described in `problems` (one line each, empty if none).
void loadTuning(const std::string& path, Tuning& t, std::string& problems);
}
