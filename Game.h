#ifndef GAME_H
#define GAME_H

#include "DefaultVertex.h"
#include "Flipper.h"

#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>

#include <cassert>
#include <vector>

constexpr float pi{ glm::pi<float>() };
constexpr float twoPi{ 2.0f * pi };

inline glm::vec2 perp(glm::vec2 v)
{
    return {-v.y, v.x};
}

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
