#ifndef MATH_H
#define MATH_H

#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>

constexpr float pi{ glm::pi<float>() };
constexpr float twoPi{ 2.0f * pi };

inline glm::vec2 perp(glm::vec2 v)
{
    return {-v.y, v.x};
}

#endif
