#include "game.h"

Scene makeScene()
{
    Scene scene{};

    scene.circle = { {0.0f, 10.0f}, 1.0f };

    constexpr float flipperX{ 10.0f };
    constexpr float flipperY{ 7.0f };

    scene.flippers.emplace_back(glm::vec2{ -flipperX, flipperY }, true);
    scene.flippers.emplace_back(glm::vec2{  flipperX, flipperY }, false);

    glm::vec2 p0{ -flipperX - 0.5f, flipperY + Flipper::r0 + 0.5f };
    scene.lines.emplace_back(Line{ p0, p0 + glm::vec2{ -10.0f, 10.0f } });

    return scene;
}

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
