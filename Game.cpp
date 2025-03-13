#include "Game.h"

void Game::update(float dt, std::uint8_t input)
{
    if (input & BUTTON_L)
    {
        flippers[0].activate();
    }
    else
    {
        flippers[0].deactivate();
    }

    if (input & BUTTON_R)
    {
        flippers[1].activate();
    }
    else
    {
        flippers[1].deactivate();
    }

    for (auto& flipper : flippers)
    {
        flipper.update(dt);
    }
}
