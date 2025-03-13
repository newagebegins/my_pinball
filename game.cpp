#include "game.h"

void update(Scene& scene, float dt, std::uint8_t input)
{
    if (input & BUTTON_L)
    {
        scene.flippers[0].activate();
    }
    else
    {
        scene.flippers[0].deactivate();
    }

    if (input & BUTTON_R)
    {
        scene.flippers[1].activate();
    }
    else
    {
        scene.flippers[1].deactivate();
    }

    for (auto& flipper : scene.flippers)
    {
        flipper.update(dt);
    }
}
