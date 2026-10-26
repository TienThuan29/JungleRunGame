#pragma once
#include "core/Game.h"

namespace jr {
// save.txt next to the exe (current folder if SDL cannot tell): `key value` lines.
// Missing file or bad values -> defaults. Saving writes save.txt.tmp, then renames it over save.txt.
void loadSettings(Settings& s);
void saveSettings(const Settings& s);
}
