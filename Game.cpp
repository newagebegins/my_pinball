#include "Game.h"

#include "Constants.h"

glm::vec2 perp(glm::vec2 v)
{
    return {-v.y, v.x};
}

struct Line
{
    glm::vec2 p; // point on the line
    glm::vec2 d; // direction

    Line(glm::vec2 P, glm::vec2 D) : p{P}, d{D}
    {}

    Line(glm::vec2 P, float a) : p{P}, d{ std::cos(a), std::sin(a) }
    {}

    Line parallel(float offset)
    {
        glm::vec2 n{ perp(d) };
        return {{p + n*offset}, d};
    }

    static Line vertical(float x)
    {
        return {{x, 0.0f}, {0.0f, 1.0f}};
    }

    static Line horizontal(float y)
    {
        return {{0.0f, y}, {1.0f, 0.0f}};
    }
};

constexpr glm::vec3 defCol{ 1.0f, 1.0f, 1.0f };
constexpr glm::vec3 auxCol{ 0.5f, 0.5f, 0.5f };
constexpr glm::vec3 highlightCol{ 0.8f, 0.0f, 0.3f };

void addLineSegment(std::vector<DefaultVertex>& verts, glm::vec2 p0, glm::vec2 p1)
{
    verts.push_back({p0, defCol});
    verts.push_back({p1, defCol});
}

void addMirroredLineSegments(std::vector<DefaultVertex>& verts, glm::vec2 p0, glm::vec2 p1, glm::vec3 color = defCol)
{
    verts.push_back({p0, color});
    verts.push_back({p1, color});

    verts.push_back({{-p0.x, p0.y}, color});
    verts.push_back({{-p1.x, p1.y}, color});
}

glm::vec2 findIntersection(Line L1, Line L2)
{
    const float p1x{ L1.p.x };
    const float p1y{ L1.p.y };
    const float d1x{ L1.d.x };
    const float d1y{ L1.d.y };

    const float p2x{ L2.p.x };
    const float p2y{ L2.p.y };
    const float d2x{ L2.d.x };
    const float d2y{ L2.d.y };

    const float num{ d2x*(p2y - p1y) + d2y*(p1x - p2x)};
    const float denom{ d1y*d2x - d2y*d1x };
    const float t1{ num / denom };

    return L1.p + L1.d * t1;
}

void addLine(std::vector<DefaultVertex>& verts, const Line& l, glm::vec3 color = auxCol)
{
    constexpr float len{ 100.0f };
    verts.push_back({l.p + l.d*len, color});
    verts.push_back({l.p - l.d*len, color});
}

Game::Game()
{
    // Ball's radius is 1.0f, everything is measured relative to that

    circle = { {0.0f, 10.0f}, 1.0f };

    constexpr float flipperX{ 10.0f };
    constexpr float flipperY{ 7.0f };

    flippers.emplace_back(glm::vec2{ -flipperX, flipperY }, true);
    flippers.emplace_back(glm::vec2{  flipperX, flipperY }, false);

    const glm::vec2 p0{ -flipperX - 0.5f, flipperY + Flipper::r0 + 0.5f };

    Line l0{ p0, Flipper::minAngle };
    Line l1{ Line::vertical(-flipperX - 9.0f) };
    addLine(lines, l0);
    addLine(lines, l1);

    const glm::vec2 p1{ findIntersection(l0, l1) };
    const glm::vec2 p2{ p1 + glm::vec2{ 0.0f, 13.0f } };

    // Angled wall right near the flipper
    addMirroredLineSegments(lines, p0, p1);
    addMirroredLineSegments(lines, p1, p2);

    Line l2{ l0.parallel(-5.0f) };
    addLine(lines, l2);

    Line l3{ l1.parallel(4.0f) };
    addLine(lines, l3);

    Line l4{ Line::vertical(-flipperX - 4.5f) };
    addLine(lines, l4);

    Line worldB{ Line::horizontal(Constants::worldB) };
    
    const glm::vec2 p3{ findIntersection(l4, worldB) };
    const glm::vec2 p4{ findIntersection(l2, l4) };
    const glm::vec2 p5{ findIntersection(l2, l3) };
    const glm::vec2 p6{ p5.x, p5.y + 14.0f };

    // Outer wall near the flipper
    addMirroredLineSegments(lines, p3, p4);
    addMirroredLineSegments(lines, p4, p5);
    addMirroredLineSegments(lines, p5, p6);

    // Draw a border that represents the gameplay area
    {
        constexpr float d{ 0.1f };
        // bottom
        addLine(lines, Line::horizontal(Constants::worldB + d));
        // top
        addLine(lines, Line::horizontal(Constants::worldT - d));
        // left
        addLine(lines, Line::vertical(Constants::worldL + d));
        // right
        addLine(lines, Line::vertical(Constants::worldR - d));
    }
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
