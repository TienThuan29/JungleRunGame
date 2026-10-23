#pragma once
#include "game/Entity.h"
#include <memory>

namespace jr {
// Tile code (see the legend in Resource/Levels/lvNN.txt) at cell top-left (x, y) -> entity; code 0 -> nullptr.
std::unique_ptr<Entity> makeEntity(int code, float x, float y);
}
