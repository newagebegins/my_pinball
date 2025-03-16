#include "Game.h"

#include "Constants.h"
#include "Math.h"

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
[[maybe_unused]] constexpr glm::vec3 highlightCol{ 0.8f, 0.0f, 0.3f };

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

void addLineStrip(std::vector<DefaultVertex>& verts, const std::vector<glm::vec2>& pts, float xScale = 1.0f, glm::vec3 color = defCol)
{
    std::size_t len = pts.size();
    assert(len > 1);
    verts.push_back({ {pts[0].x*xScale, pts[0].y}, color });
    for (std::size_t i{1}; i < len-1; ++i)
    {
        DefaultVertex v{{pts[i].x*xScale, pts[i].y}, color};
        verts.push_back(v);
        verts.push_back(v);
    }
    verts.push_back({ {pts[len-1].x*xScale, pts[len-1].y}, color });
}

void addLineStripMirrored(std::vector<DefaultVertex>& verts, const std::vector<glm::vec2>& pts, glm::vec3 color = defCol)
{
    addLineStrip(verts, pts, 1.0f, color);
    addLineStrip(verts, pts, -1.0f, color);
}

void addCircleLines(std::vector<DefaultVertex>& verts, Circle c)
{
    constexpr int numVerts{ 32 };
    glm::vec3 color{ 1.0f, 1.0f, 1.0f };

    DefaultVertex v0{
        c.center + glm::vec2{ 1.0f, 0.0f } * c.radius,
        color,
    };

    verts.push_back(v0);

    for (std::size_t i{ 1 }; i < numVerts; ++i)
    {
        const float t{ static_cast<float>(i) / numVerts };
        const float angle{ t * twoPi };
        const DefaultVertex v {
            c.center + glm::vec2{ std::cos(angle), std::sin(angle) } * c.radius,
            color,
        };
        verts.push_back(v);
        verts.push_back(v);
    }

    verts.push_back(v0);
}

// P - intersection of two lines
// d1 - direction of the line to the left of the circle (from intersection towards circle, unit)
// d2 - direction of the line to the right of the circle (from intersection towards circle, unit)
// r - radius of the circle
// returns the position of the circle
glm::vec2 findCircleBetweenLines(glm::vec2 P, glm::vec2 d1, glm::vec2 d2, float r)
{
    glm::vec2 d1p = perp(d1);
    glm::vec2 d2p = perp(d2);
    float t = r * glm::length(d1p + d2p) / glm::length(d1 - d2);
    glm::vec2 O = P + d1*t - d1p*r;
    return O;
}

float getAngle(glm::vec2 v)
{
    float a = std::atan2(v.y, v.x);
    if (a < 0) a += twoPi;
    return a;
}

// Circular through 2 points
Arc makeArc(glm::vec2 pStart, glm::vec2 pEnd, float r)
{
    glm::vec2 pMid{ (pStart + pEnd) / 2.0f };
    glm::vec2 L{ -glm::normalize(perp(pStart - pEnd)) };
    float m{ glm::length(pMid-pEnd) };
    float l{ std::sqrt(r*r - m*m) };
    glm::vec2 c{pMid + L*l};
    float start{ getAngle(pStart - c) };
    float end{ getAngle(pEnd - c) };
    return {c, r, start, end};
}

struct ArcPoints
{
    glm::vec2 pStart;
    glm::vec2 pEnd;
};

// P - intersection of two lines
// d1 - direction of the line to the left of the circle (from intersection towards circle, unit)
// d2 - direction of the line to the right of the circle (from intersection towards circle, unit)
// r - radius of the circle
// returns the position of the circle
ArcPoints findArcBetweenLines(glm::vec2 P, glm::vec2 d1, glm::vec2 d2, float r)
{
    glm::vec2 d1p = perp(d1);
    glm::vec2 d2p = perp(d2);
    float t = r * glm::length(d1p + d2p) / glm::length(d1 - d2);
    glm::vec2 O = P + d1*t - d1p*r;
    glm::vec2 Q = P + d1*t;
    glm::vec2 R = P + d2*t;
    return {Q, R};
}

std::vector<glm::vec2> getArcPoints(const Arc& arc, int numSteps = 32)
{
    std::vector<glm::vec2> verts{};

    float start{ arc.start };
    float end{ arc.end };
    if (arc.end < arc.start)
    {
        end += twoPi;
    }

    for (int i{ 0 }; i < numSteps; ++i)
    {
        const float t{ static_cast<float>(i) / (numSteps - 1) };
        const float angle{ glm::mix(start, end, t) };
        float x = arc.p.x + std::cos(angle) * arc.r;
        float y = arc.p.y + std::sin(angle) * arc.r;
        verts.push_back({x,y});
    }

    return verts;
}

void addArcLines(std::vector<DefaultVertex>& verts, const Arc& arc, int numSteps = 32, glm::vec3 color = defCol)
{
    auto pts{ getArcPoints(arc, numSteps) };
    addLineStrip(verts, pts, 1.0f, color);
}

void addArcLinesMirrored(std::vector<DefaultVertex>& verts, const Arc& arc, int numSteps = 32, glm::vec3 color = defCol)
{
    addLineStripMirrored(verts, getArcPoints(arc, numSteps), color);
}

void addSlingshotCircle(std::vector<DefaultVertex>& verts, glm::vec2 P, glm::vec2 d1, glm::vec2 d2, float r)
{
    glm::vec2 O = findCircleBetweenLines(P, d1, d2, r);
    Circle c{ O, r };
    addCircleLines(verts, c);
}

Game::Game()
{
    // Ball's radius is 1.0f, everything is measured relative to that

    circles.push_back({ {0.0f, 10.0f}, 1.0f });

    constexpr float flipperX{ 10.0f };
    constexpr float flipperY{ 7.0f };

    flippers.emplace_back(glm::vec2{ -flipperX, flipperY }, true);
    flippers.emplace_back(glm::vec2{  flipperX, flipperY }, false);

    const glm::vec2 p0{ -flipperX - 0.5f, flipperY + Flipper::r0 + 0.5f };

    Line l0{ p0, Flipper::minAngle };
    Line l1{ Line::vertical(-flipperX - 9.0f) };
    // addLine(lines, l0);
    // addLine(lines, l1);

    const glm::vec2 p1{ findIntersection(l0, l1) };
    const glm::vec2 p2{ p1 + glm::vec2{ 0.0f, 13.0f } };

    // Angled wall right near the flipper
    addLineStripMirrored(lines, {p0,p1,p2});

    Line l2{ l0.parallel(-5.0f) };
    // addLine(lines, l2);

    Line l3{ l1.parallel(4.0f) };
    // addLine(lines, l3);

    Line l4{ Line::vertical(-flipperX - 4.5f) };
    // addLine(lines, l4);

    Line worldB{ Line::horizontal(Constants::worldB) };
    
    // ditch
    glm::vec2 pp1 = findIntersection(l1, l2);
    Line ll1 = Line::horizontal(pp1.y - 3.0f);
    glm::vec2 pp2 = findIntersection(l1, ll1);
    glm::vec2 pp3 = findIntersection(ll1, l3);

    const glm::vec2 p3{ findIntersection(l4, worldB) };
    const glm::vec2 p4{ findIntersection(l2, l4) };
    const glm::vec2 p6{ pp3.x, pp3.y + 20.0f };

    // Outer wall near the flipper
    addLineStripMirrored(lines, {p3,p4,pp1,pp2,pp3,p6});

    //
    // Slingshot
    //

    Line sL{ l1.parallel(-3.0f) };
    // addLine(lines, sL);
    Line sB{ l0.parallel(3.5f) };
    // addLine(lines, sB);
    glm::vec2 sLB{ findIntersection(sL, sB) };
    Line sLB1{ sLB, glm::radians(109.0f) };
    // addLine(lines, sLB1, highlightCol);
    Line sR{ sLB1.parallel(-4.0f) };
    // addLine(lines, sR);
    glm::vec2 sRB{ findIntersection(sR, sB) };
    glm::vec2 sLR{ findIntersection(sL, sR) };

    // Construct slingshot
    {
        float LRr = 0.8f;
        ArcPoints apLR = findArcBetweenLines(sLR, -sR.d, -sL.d, LRr);
        addArcLinesMirrored(lines, makeArc(apLR.pStart, apLR.pEnd, LRr), 8);

        float RBr = 0.82f;
        ArcPoints apRB = findArcBetweenLines(sRB, -sB.d, sR.d, RBr);
        addArcLinesMirrored(lines, makeArc(apRB.pStart, apRB.pEnd, RBr), 8);

        float LBr = 2.0f;
        ArcPoints apLB = findArcBetweenLines(sLB, sL.d, sB.d, LBr);
        addArcLinesMirrored(lines, makeArc(apLB.pStart, apLB.pEnd, LBr), 8);

        addMirroredLineSegments(lines, apLR.pStart, apRB.pEnd);
        addMirroredLineSegments(lines, apRB.pStart, apLB.pEnd);
        addMirroredLineSegments(lines, apLB.pStart, apLR.pEnd);
    }

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

    glm::vec2 p7{p2 + glm::vec2{2.0f, 7.0f}};

    addArcLines(lines, makeArc(p7, p6, 10.0f), 8);
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

// Circle through 2 points with the given radius
Circle makeCircle(glm::vec2 p1, glm::vec2 p2, float r)
{
    glm::vec2 p3{ (p1 + p2) / 2.0f };
    glm::vec2 L{ -glm::normalize(perp(p2 - p1)) };
    float m{ glm::length(p3-p1) };
    float l{ std::sqrt(r*r - m*m) };
    glm::vec2 c{p3 + L*l};
    return {c, r};
}
