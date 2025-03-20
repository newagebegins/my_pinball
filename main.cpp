#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp> // for glm::ortho()
#include <glm/ext/scalar_constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

struct RenderData
{
    GLuint program;

    GLint modelLoc;
    GLint viewLoc;
    GLint projectionLoc;

    GLuint lineVao;
    int numLineVerts;

    GLuint circleVao;
    GLuint flipperVao;
};

namespace Constants
{
    constexpr float worldSize{ 70.0f };
    constexpr float worldL{ -worldSize/2.0f };
    constexpr float worldR{ worldSize/2.0f };
    constexpr float worldT{ worldSize };
    constexpr float worldB{ 0.0f };
}

struct DefaultVertex
{
    glm::vec2 pos;
    glm::vec3 col;
};

constexpr float pi{ glm::pi<float>() };
constexpr float twoPi{ 2.0f * pi };

static glm::vec2 perp(glm::vec2 v)
{
    return {-v.y, v.x};
}

#define BUTTON_L (1 << 0)
#define BUTTON_R (1 << 1)

struct Circle
{
    glm::vec2 center;
    float radius;
};

struct Arc
{
    glm::vec2 p;
    float r;
    float start;
    float end;

    Arc(glm::vec2 P, float R, float S, float E)
        : p{P}, r{R}, start{S}, end{E}
    {
        assert(0.0f <= start && start < twoPi);
        assert(0.0f <= end && end < twoPi);    
    }
};

class Flipper
{
public:
    static constexpr float r0{ 1.1f };
    static constexpr float r1{ 0.7f };

    static constexpr float minAngle{ glm::radians(-38.0f) };
    static constexpr float maxAngle{ glm::radians(33.0f) };

    Flipper(glm::vec2 position, bool isLeft)
        : m_position{ position }
    , m_scaleX{ isLeft ? 1.0f : -1.0f }
    {
        updateTransform();
    }

    void activate()
    {
        m_angularVelocity = maxAngularVelocity;
    }

    void deactivate()
    {
        m_angularVelocity = -maxAngularVelocity;
    }

    void update(float dt)
    {
        m_orientation += m_angularVelocity * dt;
        m_orientation = glm::clamp(m_orientation, minAngle, maxAngle);
        updateTransform();
    }

    const glm::mat3& getTransform() const
    {
        return m_transform;
    }

private:
    static constexpr float maxAngularVelocity{ 2.0f * glm::pi<float>() * 4.0f };

    glm::mat3 m_transform{ glm::mat3{ 1.0f } };
    const glm::vec2 m_position{ glm::vec2{ 0.0f } };
    float m_orientation{ 0.0f };
    const float m_scaleX{ 1.0f };
    float m_angularVelocity{ -maxAngularVelocity };

    void updateTransform()
    {
        m_transform = glm::mat3{ 1.0f };
        m_transform = glm::translate(m_transform, m_position);
        m_transform = glm::rotate(m_transform, m_orientation * m_scaleX);
        m_transform = glm::scale(m_transform, { m_scaleX, 1.0f });
    }
};

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

struct Ray
{
    glm::vec2 p;
    glm::vec2 d;
};

constexpr glm::vec3 defCol{ 1.0f, 1.0f, 1.0f };
constexpr glm::vec3 auxCol{ 0.5f, 0.5f, 0.5f };
[[maybe_unused]] constexpr glm::vec3 highlightCol{ 0.8f, 0.0f, 0.3f };

void addLineSegment(std::vector<DefaultVertex>& verts, glm::vec2 p0, glm::vec2 p1)
{
    verts.push_back({p0, defCol});
    verts.push_back({p1, defCol});
}

void addLineSegmentMirrored(std::vector<DefaultVertex>& verts, glm::vec2 p0, glm::vec2 p1, glm::vec3 color = defCol)
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

void addRay(std::vector<DefaultVertex>& verts, const Ray& r, glm::vec3 color = auxCol)
{
    constexpr float len{ 100.0f };
    verts.push_back({r.p, color});
    verts.push_back({r.p + r.d*len, color});
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
    float l{ std::abs(r-m) < 0.001f ? 0.0f : std::sqrt(r*r - m*m) };
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

glm::vec2 makeVec(float angle, float len)
{
    return { std::cos(angle) * len, std::sin(angle) * len };
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

glm::vec2 getArcStart(const Arc& arc)
{
    return arc.p + glm::vec2{std::cos(arc.start), std::sin(arc.start)} * arc.r;
}

glm::vec2 getArcEnd(const Arc& arc)
{
    return arc.p + glm::vec2{std::cos(arc.end), std::sin(arc.end)} * arc.r;
}

glm::vec2 findIntersection(const Ray& r, const Arc& a)
{
    float n = r.p.x - a.p.x;
    float m = r.p.y - a.p.y;
    float b = 2.0f*(r.d.x*n + r.d.y*m);
    float c = n*n + m*m - a.r*a.r;
    float D = b*b - 4*c;
    float t = (-b + std::sqrt(D)) / 2.0f;
    return r.p + r.d * t;
}

void addCapsuleLines(std::vector<DefaultVertex>& verts, glm::vec2 c)
{
    float hw=0.45f;
    float hh=1.0f;
    glm::vec2 tl{c.x-hw,c.y+hh};
    glm::vec2 tr{c.x+hw,c.y+hh};
    glm::vec2 bl{c.x-hw,c.y-hh};
    glm::vec2 br{c.x+hw,c.y-hh};
    addLineSegment(verts, tl, bl);
    addLineSegment(verts, tr, br);
    int s = 8;
    addArcLines(verts, makeArc(tr, tl, hw), s);
    addArcLines(verts, makeArc(bl, br, hw), s);
}

void addPopBumperLines(std::vector<DefaultVertex>& verts, glm::vec2 c)
{
    float rb{2.75f};
    float gap{0.45f};
    float rs{rb-gap};
    addCircleLines(verts, {c, rb});
    addCircleLines(verts, {c, rs});
}

void addButton(std::vector<DefaultVertex>& verts, glm::vec2 p0, glm::vec2 p1, float t)
{
    glm::vec2 D{p1-p0};
    glm::vec2 c{ p0 + D*t };
    glm::vec2 d{glm::normalize(D)};
    glm::vec2 dp = perp(d);
    float hw = 1.4f;
    float h = 0.6f;
    glm::vec2 q0 = c - d*hw;
    glm::vec2 q3 = c + d*hw;
    glm::vec2 q1 = q0 + dp*h;
    glm::vec2 q2 = q3 + dp*h;
    addLineStrip(verts, {q0,q1,q2,q3});
}

struct Game
{
    std::vector<Circle> circles{};
    std::vector<Flipper> flippers{};
    std::vector<DefaultVertex> lines{};

    Game()
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
        addLineStripMirrored(lines, {p3,p4,pp1,pp2,pp3});
        addLineSegment(lines, pp3, p6);
    
        // vertical wall near the right flipper
        glm::vec2 pp3r = {-pp3.x, pp3.y};
        glm::vec2 p8 = {pp3r.x, pp3r.y + 23.6f};
        addLineSegment(lines, pp3r, p8);
    
        glm::vec2 p80 = findIntersection(l2, l3);
        // ditch caps
        addLineSegmentMirrored(lines, pp1, p80);
    
        // Construct slingshot
        {
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
    
            float LRr = 0.8f;
            ArcPoints apLR = findArcBetweenLines(sLR, -sR.d, -sL.d, LRr);
            addArcLinesMirrored(lines, makeArc(apLR.pStart, apLR.pEnd, LRr), 8);
    
            float RBr = 0.82f;
            ArcPoints apRB = findArcBetweenLines(sRB, -sB.d, sR.d, RBr);
            addArcLinesMirrored(lines, makeArc(apRB.pStart, apRB.pEnd, RBr), 8);
    
            float LBr = 2.0f;
            ArcPoints apLB = findArcBetweenLines(sLB, sL.d, sB.d, LBr);
            addArcLinesMirrored(lines, makeArc(apLB.pStart, apLB.pEnd, LBr), 8);
    
            addLineSegmentMirrored(lines, apLR.pStart, apRB.pEnd);
            addLineSegmentMirrored(lines, apRB.pStart, apLB.pEnd);
            addLineSegmentMirrored(lines, apLB.pStart, apLR.pEnd);
        }
    
    #if 0
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
    #endif
    
        glm::vec2 p7{p2 + glm::vec2{2.0f, 7.0f}};
    
        // left bottom arc
        addArcLines(lines, makeArc(p7, p6, 10.0f), 8);
    
        glm::vec2 p9 = {p8.x-7.5f,p8.y+10.0f};
        addArcLines(lines, makeArc(p8, p9, 11.0f), 8);
    
        Line l3r{ {-l3.p.x,l3.p.y}, l3.d };
        // addLine(lines, l3r);
        Line l20 = l3r.parallel(-0.5f);
        // addLine(lines, l20);
        float plungerShuteWidth = 3.4f;
        Line l21 = l20.parallel(-plungerShuteWidth);
        // addLine(lines, l21);
    
        glm::vec2 p20 = findIntersection(l20, worldB);
        glm::vec2 p21 = findIntersection(l21, worldB);
        float k20 = 48.0f;
        glm::vec2 p22 = p20 + glm::vec2{0.0f, 1.0f} * k20;
        glm::vec2 p23 = p21 + glm::vec2{0.0f, 1.0f} * k20;
        // Plunger shaft
        addLineSegment(lines, p20, p22);
        addLineSegment(lines, p21, p23);
    
        // Top of the plunger
        glm::vec2 p30 = findIntersection(ll1, l20);
        glm::vec2 p31 = findIntersection(ll1, l21);
        addLineSegment(lines, p30, p31);
    
        float arc30r = 20.87f;
        glm::vec2 arc30c = p23 + glm::vec2{-arc30r, 0.0f};
        Arc arc30{ arc30c, arc30r, 0.0f, glm::radians(90.0f) };
        addArcLines(lines, arc30);
    
        float arc31r = 20.87f - plungerShuteWidth;
        Arc arc31{ arc30c, arc31r, 0.0f, glm::radians(84.0f) };
        addArcLines(lines, arc31);
    
        // Right upper wall
        glm::vec2 p10 = p9 + makeVec(glm::radians(110.0f), 4.5f);
        glm::vec2 p11 = p10 + makeVec(glm::radians(31.0f), 5.3f);
        glm::vec2 p12 = p11 + makeVec(glm::radians(97.0f), 12.2f);
        glm::vec2 p13 = p12 + makeVec(glm::radians(150.0f), 10.85f);
        glm::vec2 p14 = getArcEnd(arc31);
        addLineStrip(lines, {p9,p10,p11,p12,p13,p14});
    
        addButton(lines, p9, p10, 0.5f);
        addButton(lines, p10, p11, 0.5f);
        addButton(lines, p11, p12, 0.3f);
        addButton(lines, p11, p12, 0.7f);
        addButton(lines, p12, p13, 0.5f);
    
        Ray r30{ p14, glm::normalize(p14-p13) };
        glm::vec2 p15 = findIntersection(r30, arc30);
        // right one-way wall
        addLineSegment(lines, p14, p15);
    
        glm::vec2 p40 = getArcEnd(arc30);
        glm::vec2 p41 = p40 + glm::vec2{-7.68f, 0.0f};
        // bridge between left and right arcs at the top of the table
        addLineSegment(lines, p40, p41);
        // addCircleLines(lines, {p41, 0.5f});
    
        // left top big arc
        Arc a50 = makeArc(p41, p7, 20.8f);
        addArcLines(lines, a50);
    
        // left small arc
        Arc a51 = {a50.p, a50.r - plungerShuteWidth, glm::radians(105.0f), glm::radians(130.0f)};
        addArcLines(lines, a51);
    
        // left medium arc
        Arc a52 = {a50.p, a50.r - plungerShuteWidth, glm::radians(150.0f), glm::radians(205.0f)};
        addArcLines(lines, a52);
    
        glm::vec2 a51s = getArcStart(a51);
        Ray r51s{ a51.p, glm::normalize(a51s - a51.p) };
        // addRay(lines, r51s); 
    
        glm::vec2 a51e = getArcEnd(a51);
        Ray r51e{ a51.p, glm::normalize(a51e - a51.p) };
        // addRay(lines, r51e);
    
        glm::vec2 p50 = findIntersection(r51s, a50);
        // left one-way wall
        addLineSegment(lines, a51s, p50);
    
        float w51 = 2.3f;
        glm::vec2 p53 = a51s - r51s.d*w51;
        glm::vec2 p54 = a51e - r51e.d*w51;
    
        // left-top walled island
        addLineStrip(lines, {a51s,p53,p54,a51e});
        addButton(lines, p53, p54, 0.5f);
    
        glm::vec2 a52s = getArcStart(a52);
        glm::vec2 a52e = getArcEnd(a52);
        glm::vec2 p60 = a52e + makeVec(glm::radians(-32.5f), 3.6f);
        glm::vec2 p61 = p60 + makeVec(glm::radians(44.0f), 4.5f);
        glm::vec2 p62 = p61 + makeVec(glm::radians(167.6f), 4.3f);
        // left-middle walled island
        addLineStrip(lines, {a52e,p60,p61,p62,a52s});
        addButton(lines, p61, p60, 0.5f);
        addButton(lines, p62, p61, 0.5f);
        addButton(lines, a52s, p62, 0.3f);
        addButton(lines, a52s, p62, 0.7f);
    
        float capsuleGap = 4.3f;
        float leftCapsuleX = -0.7f;
        float rightCapsuleX = leftCapsuleX + capsuleGap;
        float capsuleY = p53.y;
        addCapsuleLines(lines, {leftCapsuleX, capsuleY});
        addCapsuleLines(lines, {rightCapsuleX, capsuleY});
    
        // Central vertical symmetry line
        //addLine(lines, Line{ {0.0f,0.0f}, {0.0f,1.0f} });
    
        glm::vec2 pb1{-4.0f, 53.0f};
        glm::vec2 pb2{pb1.x+10.7f, pb1.y+0.5f};
        glm::vec2 pb3{pb1.x+5.5f, pb1.y-7.5f};
        addPopBumperLines(lines, pb1);
        addPopBumperLines(lines, pb2);
        addPopBumperLines(lines, pb3);
    }

    void update(float dt, std::uint8_t input)
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
};

namespace File
{
    static std::string readEntireFile(const char* path)
    {
        std::ifstream ifs{ path };
        if (!ifs.is_open())
        {
            std::cerr << "Failed to open the file: " << path << '\n';
            std::exit(EXIT_FAILURE);
        }
        std::stringstream s{};
        s << ifs.rdbuf();
        return s.str();
    }
}

class Shader
{
public:
    const GLuint id{};
    Shader(GLenum type, const char* path)
        : id{ glCreateShader(type) }
    {
        std::string codeStr{ File::readEntireFile(path) };
        const char* code{ codeStr.c_str() };

        glShaderSource(id, 1, &code, nullptr);
        glCompileShader(id);

        GLint success{};
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            GLchar infoLog[512];
            glGetShaderInfoLog(id, sizeof(infoLog), nullptr, infoLog);
            switch (type)
            {
                case GL_VERTEX_SHADER:   std::cerr << "Vertex"; break;
                case GL_FRAGMENT_SHADER: std::cerr << "Fragment"; break;
                default:                 std::cerr << "???"; break;
            }
            std::cerr << " shader error:\n" << infoLog << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    Shader(const Shader&) = delete;

    ~Shader()
    {
        glDeleteShader(id);
    }
};

static GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath)
{
    const Shader vs{ GL_VERTEX_SHADER, vertexPath };
    const Shader fs{ GL_FRAGMENT_SHADER, fragmentPath };

    const GLuint id{ glCreateProgram() };
    glAttachShader(id, vs.id);
    glAttachShader(id, fs.id);
    glLinkProgram(id);

    GLint success{};
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetProgramInfoLog(id, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Program link error:\n" << infoLog << '\n';
        std::exit(EXIT_FAILURE);
    }

    return id;
}

static GLuint createVao(const std::vector<DefaultVertex>& verts)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    const GLsizeiptr size{ static_cast<GLsizeiptr>(verts.size() * sizeof(verts[0])) };
    glBufferData(GL_ARRAY_BUFFER, size, verts.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(verts[0]), (void *)offsetof(DefaultVertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(verts[0]), (void *)offsetof(DefaultVertex, col));

    return vao;
}

constexpr int numFlipperCircleSegments1{ 16 };
constexpr int numFlipperCircleSegments2{ 8 };
constexpr int numFlipperVerts{ (numFlipperCircleSegments1 + 1) + (numFlipperCircleSegments2 + 1) };

static std::vector<DefaultVertex> makeFlipperVerts()
{
    constexpr float width{ 8.0f };
    constexpr float d{ width - Flipper::r0 - Flipper::r1 };
    constexpr float cosA{ (Flipper::r0 - Flipper::r1) / d };
    const float a{ std::acos(cosA) };
    constexpr glm::vec3 color{ 1.0f, 1.0f, 1.0f };

    std::vector<DefaultVertex> verts(numFlipperVerts);

    std::size_t nextVert{ 0 };

    for (int i{ 0 }; i <= numFlipperCircleSegments1; ++i)
    {
        const float t{ static_cast<float>(i) / numFlipperCircleSegments1 };
        const float angle{ a + 2.0f * t * (glm::pi<float>() - a) };
        const float x{ Flipper::r0 * std::cos(angle) };
        const float y{ Flipper::r0 * std::sin(angle) };
        verts[nextVert++] = { { x, y }, color };
    }

    for (int i{ 0 }; i <= numFlipperCircleSegments2; ++i)
    {
        const float t{ static_cast<float>(i) / numFlipperCircleSegments2 };
        const float angle{ -a + t * 2.0f * a };
        const float x{ d + Flipper::r1 * std::cos(angle) };
        const float y{ Flipper::r1 * std::sin(angle) };
        verts[nextVert++] = { { x, y }, color };
    }

    assert(nextVert == numFlipperVerts);

    return verts;
}

constexpr int numCircleVerts{ 64 };

static std::vector<DefaultVertex> makeCircleVerts()
{
    std::vector<DefaultVertex> verts(numCircleVerts);

    for (std::size_t i{ 0 }; i < numCircleVerts; ++i)
    {
        const float t{ static_cast<float>(i) / numCircleVerts };
        const float angle{ t * 2.0f * glm::pi<float>() };
        verts[i] = {
            { std::cos(angle), std::sin(angle) },
            { 1.0f, 1.0f, 1.0f },
        };
    }

    return verts;
}

static Game game;
static RenderData rd;

static void render()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (const auto& c : game.circles)
    {
        glm::mat3 model{ 1.0f };
        model = glm::translate(model, c.center);
        model = glm::scale(model, glm::vec2{ c.radius });
        glUniformMatrix3fv(rd.modelLoc, 1, GL_FALSE, &model[0][0]);

        glBindVertexArray(rd.circleVao);
        glDrawArrays(GL_LINE_LOOP, 0, numCircleVerts);
    }

    for (const auto& flipper : game.flippers)
    {
        glUniformMatrix3fv(rd.modelLoc, 1, GL_FALSE, &flipper.getTransform()[0][0]);
        glBindVertexArray(rd.flipperVao);
        glDrawArrays(GL_LINE_LOOP, 0, numFlipperVerts);
    }

    glUniformMatrix3fv(rd.modelLoc, 1, GL_FALSE, &glm::mat3{1.0f}[0][0]);
    glBindVertexArray(rd.lineVao);
    glDrawArrays(GL_LINES, 0, rd.numLineVerts);
}

static void APIENTRY glDebugOutput(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei /*length*/,
    const GLchar* message,
    const void* /*userParam*/)
{
    // Skip uninteresting messages
    if (id == 131185) // Buffer object will use VIDEO memory as the source for buffer object operations
        return;

    std::cerr << "OpenGL debug message (" << id << "): " << message << '\n';

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cerr << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cerr << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cerr << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cerr << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cerr << "Source: Other"; break;
    default:                              std::cerr << "Source: ???"; break;
    }
    std::cerr << '\n';

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cerr << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cerr << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cerr << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cerr << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cerr << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cerr << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cerr << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cerr << "Type: Other"; break;
    default:                                std::cerr << "Type: ???"; break;
    }
    std::cerr << '\n';

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cerr << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cerr << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cerr << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "Severity: notification"; break;
    default:                             std::cerr << "Severity: ???"; break;
    }
    std::cerr << "\n\n";
}

static void errorCallback(int /*error*/, const char* description)
{
    std::cerr << "GLFW error: " << description << '\n';
}

static void framebufferSizeCallback(GLFWwindow* /*window*/, int width, int height)
{
    if (width > height)
    {
        int w{ height };
        glViewport(width/2 - w/2, 0, w, height);
    }
    else
    {
        int h{ width };
        glViewport(0, height/2 - h/2, width, h);
    }
}

static void windowRefreshCallback(GLFWwindow* window)
{
    render();
    glfwSwapBuffers(window);
}

int main()
{
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    GLFWwindow* window{ glfwCreateWindow(800, 800, "my_pinball", nullptr, nullptr) };
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return EXIT_FAILURE;
    }

    if (GLAD_GL_KHR_debug)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetWindowRefreshCallback(window, windowRefreshCallback);

    rd.program = createShaderProgram("assets/default_v.glsl", "assets/default_f.glsl");

    rd.modelLoc = glGetUniformLocation(rd.program, "model");
    rd.viewLoc = glGetUniformLocation(rd.program, "view");
    rd.projectionLoc = glGetUniformLocation(rd.program, "projection");

    assert(rd.modelLoc >= 0);
    assert(rd.viewLoc >= 0);
    assert(rd.projectionLoc >= 0);

    rd.lineVao = createVao(game.lines);
    rd.numLineVerts = static_cast<int>(game.lines.size());

    rd.circleVao = createVao(makeCircleVerts());
    rd.flipperVao = createVao(makeFlipperVerts());

    const glm::mat3 identity{ 1.0f };
    const glm::mat4 projection{ glm::ortho(Constants::worldL, Constants::worldR, Constants::worldB, Constants::worldT, -1.0f, 1.0f) };

    glUseProgram(rd.program);
    glUniformMatrix3fv(rd.viewLoc, 1, GL_FALSE, &identity[0][0]);
    glUniformMatrix4fv(rd.projectionLoc, 1, GL_FALSE, &projection[0][0]);

    while (!glfwWindowShouldClose(window))
    {
        static float prevTime{ static_cast<float>(glfwGetTime()) };
        const float currentTime{ static_cast<float>(glfwGetTime()) };
        const float dt = currentTime - prevTime;
        prevTime = currentTime;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        std::uint8_t input{};

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            input |= BUTTON_L;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            input |= BUTTON_R;
        }

        game.update(dt, input);
        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return EXIT_SUCCESS;
}
