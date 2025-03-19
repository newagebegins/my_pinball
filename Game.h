#ifndef GAME_H
#define GAME_H

#include "DefaultVertex.h"
#include "Flipper.h"
#include "Math.h"

#include <glm/glm.hpp>

#include <cassert>
#include <vector>

#define BUTTON_L (1 << 0)
#define BUTTON_R (1 << 1)

struct Circle
{
    glm::vec2 center;
    float radius;
};

struct Arc
{
    glm::vec2 p;
    float r;
    float start;
    float end;

    Arc(glm::vec2 P, float R, float S, float E)
        : p{P}, r{R}, start{S}, end{E}
    {
        assert(0.0f <= start && start < twoPi);
        assert(0.0f <= end && end < twoPi);    
    }
};

struct Game
{
    std::vector<Circle> circles{};
    std::vector<Flipper> flippers{};
    std::vector<DefaultVertex> lines{};

    Game();
    void update(float dt, std::uint8_t input);
};

#endif
