#ifndef GAME_H
#define GAME_H

#include "Circle.h"
#include "DefaultVertex.h"
#include "Flipper.h"

#include <glm/glm.hpp>

#include <vector>

#define BUTTON_L (1 << 0)
#define BUTTON_R (1 << 1)

struct Game
{
    std::vector<Circle> circles{};
    std::vector<Flipper> flippers{};
    std::vector<DefaultVertex> lines{};

    Game();
    void update(float dt, std::uint8_t input);
private:
    void addCirc(glm::vec2 p);
    void addCirc(glm::vec2 p1, glm::vec2 p2, float r);
};

#endif
