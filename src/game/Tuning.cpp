#include "game/Tuning.h"
#include <charconv>
#include <fstream>
#include <sstream>

namespace jr {
namespace {
struct Key {
	const char* name;
	float Tuning::*field;
	float lo, hi;
};

const Key kKeys[] = {
	{ "run_accel", &Tuning::runAccel, 0.01f, 5 },
	{ "run_max", &Tuning::runMax, 0.5f, 20 },
	{ "friction", &Tuning::friction, 0, 1 },
	{ "jump_vy", &Tuning::jumpVy, -20, -1 },
	{ "gravity", &Tuning::gravity, 0.05f, 2 },
	{ "fall_accel", &Tuning::fallAccel, 0, 5 },
	{ "immortal_sec", &Tuning::immortalSec, 0.1f, 10 },
	{ "respawn_immortal_sec", &Tuning::respawnImmortalSec, 0.1f, 10 },
	{ "attack_sec", &Tuning::attackSec, 0.05f, 2 },
	{ "attack_cooldown_sec", &Tuning::attackCooldownSec, 0.05f, 5 },
	{ "attack_w", &Tuning::attackW, 10, 300 },
	{ "attack_h", &Tuning::attackH, 10, 300 },
	{ "boss_speed", &Tuning::bossSpeed, 0, 1000 },
	{ "boss_delay_sec", &Tuning::bossDelaySec, 0, 60 },
	{ "boss_fire_sec", &Tuning::bossFireSec, 0.5f, 120 },
	{ "boss_fire_range", &Tuning::bossFireRange, 0, 5000 },
	{ "boss_warn_px", &Tuning::bossWarnPx, 0, 5000 },
	{ "respawn_boss_gap", &Tuning::respawnBossGap, 300, 10000 },
	{ "arm_speed", &Tuning::armSpeed, 20, 2000 },
	{ "arrow_speed", &Tuning::arrowSpeed, 20, 2000 },
	{ "arrow_every_sec", &Tuning::arrowEverySec, 0.5f, 60 },
	{ "smol_speed", &Tuning::smolSpeed, 0, 1000 },
	{ "smol_max", &Tuning::smolMax, 0, 2000 },
	{ "smol_detect", &Tuning::smolDetect, 0, 2000 },
	{ "smol_fast_mul", &Tuning::smolFastMul, 1, 10 },
	{ "boss_hp", &Tuning::bossHp, 1, 50 },
	{ "boss_idle_sec", &Tuning::bossIdleSec, 0.1f, 20 },
	{ "boss_charge_sec", &Tuning::bossChargeSec, 0.2f, 20 },
	{ "boss_beam_sec", &Tuning::bossBeamSec, 0.1f, 20 },
	{ "boss_open_sec", &Tuning::bossOpenSec, 0.3f, 20 },
	{ "shake_hit", &Tuning::shakeHit, 0, 50 },
	{ "shake_decay", &Tuning::shakeDecay, 1, 1000 },
	{ "debug", &Tuning::debug, 0, 1 },
};
}

void loadTuning(const std::string& path, Tuning& t, std::string& problems) {
	t = Tuning{};
	problems.clear();
	std::ifstream in(path, std::ios::binary);
	if (!in) return;
	std::string line;
	for (int n = 1; std::getline(in, line); ++n) {
		if (n == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);  // UTF-8 BOM
		std::istringstream ss(line.substr(0, line.find('#')));
		std::string name, value, extra;
		if (!(ss >> name)) continue;
		const auto bad = [&](const std::string& why) { problems += "line " + std::to_string(n) + ": " + why + "\n"; };
		const Key* key = nullptr;
		for (const Key& k : kKeys) {
			if (name == k.name) key = &k;
		}
		if (!key) {
			bad("unknown key '" + name + "'");
			continue;
		}
		float v = 0;
		const bool got = static_cast<bool>(ss >> value) && !(ss >> extra);
		const char* end = value.data() + value.size();
		const auto [p, ec] = std::from_chars(value.data(), end, v);
		if (!got || ec != std::errc() || p != end) {
			bad(name + " needs one number");
			continue;
		}
		if (v < key->lo || v > key->hi) {
			bad(name + " " + value + " is outside " + std::to_string(key->lo) + " .. " + std::to_string(key->hi));
			continue;
		}
		t.*(key->field) = v;
	}
}
}
