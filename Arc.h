#ifndef ARC_H
#define ARC_H

#include "Math.h"

#include <glm/glm.hpp>

#include <cassert>

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

#endif
