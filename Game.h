#ifndef GAME_H
#define GAME_H

#include "Circle.h"
#include "Flipper.h"

#include <glm/glm.hpp>

#include <vector>

struct Game
{
    Circle circle{};
    std::vector<Flipper> flippers{};
    std::vector<glm::vec2> lines{};
};

#endif
