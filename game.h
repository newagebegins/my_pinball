#ifndef GAME_H
#define GAME_H

#include <glm/glm.hpp> // for glm types

#include <cstdint> // for std::uint8_t

struct Circle
{
    glm::vec2 center;
    float radius;
};

struct Flipper
{
    glm::mat3 transform;
    glm::vec2 position;
    float orientation;
    float scaleX;
};

struct Scene
{
    Circle circle;
    Flipper flippers[2];
};

Scene makeScene();

#define BUTTON_L (1 << 0)
#define BUTTON_R (1 << 1)

void update(std::uint8_t input);

#endif
