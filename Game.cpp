#include "Game.h"

#include "Constants.h"

constexpr glm::vec3 defCol{ 1.0f, 1.0f, 1.0f };
constexpr glm::vec3 auxCol{ 0.3f, 0.3f, 0.3f };

void addLineSegment(std::vector<DefaultVertex>& verts, glm::vec2 p0, glm::vec2 p1)
{
    verts.push_back({p0, defCol});
    verts.push_back({p1, defCol});
}

void addMirroredLineSegments(std::vector<DefaultVertex>& verts, glm::vec2 p0, glm::vec2 p1)
{
    verts.push_back({p0, defCol});
    verts.push_back({p1, defCol});

    verts.push_back({{-p0.x, p0.y}, defCol});
    verts.push_back({{-p1.x, p1.y}, defCol});
}

struct Line
{
    glm::vec2 p; // point on the line
    glm::vec2 d; // direction
};

void addLine(std::vector<DefaultVertex>& verts, const Line& l)
{
    constexpr float len{ 100.0f };
    verts.push_back({l.p + l.d*len, auxCol});
    verts.push_back({l.p - l.d*len, auxCol});
}

Game::Game()
{
    // Ball's radius is 1.0f, everything is measured relative to that

    circle = { {0.0f, 10.0f}, 1.0f };

    constexpr float flipperX{ 10.0f };
    constexpr float flipperY{ 7.0f };

    flippers.emplace_back(glm::vec2{ -flipperX, flipperY }, true);
    flippers.emplace_back(glm::vec2{  flipperX, flipperY }, false);

    // Angled wall right near the flipper
    {
        constexpr float angle{ glm::radians(180.0f) + Flipper::minAngle };
        constexpr float width{ 11.0f };
        const glm::vec2 p0{ -flipperX - 0.5f, flipperY + Flipper::r0 + 0.5f };
        const glm::vec2 p1{ p0 + glm::vec2{std::cos(angle), std::sin(angle)} * width };
        addMirroredLineSegments(lines, p0, p1);
        const glm::vec2 p2{ p1 + glm::vec2{ 0.0f, 13.0f } };
        addMirroredLineSegments(lines, p1, p2);
    }

    // Draw a border that represents the gameplay area
    {
        constexpr float d{ 0.1f };

        // bottom
        addLineSegment(lines, { Constants::worldL+d, Constants::worldB+d }, { Constants::worldR-d, Constants::worldB+d });
        // top
        addLineSegment(lines, { Constants::worldL+d, Constants::worldT-d }, { Constants::worldR-d, Constants::worldT-d });
        // left
        addLineSegment(lines, { Constants::worldL+d, Constants::worldB+d }, { Constants::worldL+d, Constants::worldT-d });
        // right
        addLineSegment(lines, { Constants::worldR-d, Constants::worldB+d }, { Constants::worldR-d, Constants::worldT-d });
    }

    addLine(lines, { { 0.0f, 0.0f }, { 0.5f, 0.5f }});
}

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
