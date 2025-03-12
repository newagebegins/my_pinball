#ifndef GAME_H
#define GAME_H

#include <glm/glm.hpp> // for glm types

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

#endif
