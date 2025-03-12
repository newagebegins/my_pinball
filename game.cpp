#include "game.h"

void addLine(std::vector<glm::vec2>& verts, glm::vec2 p0, glm::vec2 p1)
{
    verts.push_back(p0);
    verts.push_back(p1);
}

void addMirroredLines(std::vector<glm::vec2>& verts, glm::vec2 p0, glm::vec2 p1)
{
    verts.push_back(p0);
    verts.push_back(p1);

    verts.emplace_back(-p0.x, p0.y);
    verts.emplace_back(-p1.x, p1.y);
}

// Ball's radius is 1.0f, everything is measured relative to that
Scene makeScene()
{
    Scene scene{};

    scene.circle = { {0.0f, 10.0f}, 1.0f };

    constexpr float flipperX{ 10.0f };
    constexpr float flipperY{ 7.0f };

    scene.flippers.emplace_back(glm::vec2{ -flipperX, flipperY }, true);
    scene.flippers.emplace_back(glm::vec2{  flipperX, flipperY }, false);

    // Angled wall right near the flipper
    {
        constexpr float angle{ glm::radians(180.0f) + Flipper::minAngle };
        constexpr float width{ 11.0f };
        const glm::vec2 p0{ -flipperX - 0.5f, flipperY + Flipper::r0 + 0.5f };
        const glm::vec2 p1{ p0 + glm::vec2{std::cos(angle), std::sin(angle)} * width };
        addMirroredLines(scene.lines, p0, p1);
        const glm::vec2 p2{ p1 + glm::vec2{ 0.0f, 13.0f } };
        addMirroredLines(scene.lines, p1, p2);
    }

    // Horizontal line that represents the bottom of the table
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
