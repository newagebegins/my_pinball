#ifndef GAME_H
#define GAME_H

#include "Circle.h"
#include "Flipper.h"

#include <glm/glm.hpp>

#include <vector>

#define BUTTON_L (1 << 0)
#define BUTTON_R (1 << 1)

struct Game
{
    Circle circle{};
    std::vector<Flipper> flippers{};
    std::vector<glm::vec2> lines{};

    Game();
    void update(float dt, std::uint8_t input);
};

#endif
