#include "game.h"

void addLine(std::vector<glm::vec2>& verts, glm::vec2 p0, glm::vec2 p1)
{
    verts.push_back(p0);
    verts.push_back(p1);
}

Scene makeScene()
{
    Scene scene{};

    scene.circle = { {0.0f, 10.0f}, 1.0f };

    constexpr float flipperX{ 10.0f };
    constexpr float flipperY{ 7.0f };

    scene.flippers.emplace_back(glm::vec2{ -flipperX, flipperY }, true);
    scene.flippers.emplace_back(glm::vec2{  flipperX, flipperY }, false);

    constexpr float a{ glm::radians(180.0f) + Flipper::minAngle };
    const glm::vec2 p0{ -flipperX - 0.5f, flipperY + Flipper::r0 + 0.5f };
    const glm::vec2 p1{ p0 + glm::vec2{std::cos(a), std::sin(a)} * 11.0f };
    addLine(scene.lines, p0, p1);

    addLine(scene.lines, { -30.0f, 0.0f }, { 30.0f, 0.0f });

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
