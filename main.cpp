#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

constexpr float pi{ 3.14159265f };
constexpr float twoPi{ 2.0f * pi };

// Ball's radius is 1.0f, everything is measured relative to that
constexpr float ballRadius = 1.0f;

constexpr float simFps{ 120.0f };
constexpr float simDt{ 1.0f / simFps };
constexpr float minFps = 10.0f;
constexpr float maxDt = 1.0f / minFps;

struct Vec2
{
    float x;
    float y;
};

Vec2 operator*(Vec2 v, float s)
{
    return {s*v.x, s*v.y};
}

Vec2 operator*(float s, Vec2 v)
{
    return {s*v.x, s*v.y};
}

Vec2 operator/(Vec2 v, float s)
{
    return {v.x/s, v.y/s};
}

Vec2 operator+(Vec2 a, Vec2 b)
{
    return {a.x+b.x, a.y+b.y};
}

Vec2 operator-(Vec2 a, Vec2 b)
{
    return {a.x-b.x, a.y-b.y};
}

float getLength(Vec2 v)
{
    return sqrtf(v.x*v.x + v.y*v.y);
}

Vec2 normalize(Vec2 v)
{
    return v/getLength(v);
}

Vec2 operator-(Vec2 v)
{
    return {-v.x, -v.y};
}

Vec2& operator+=(Vec2& a, Vec2 b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

float dot(Vec2 a, Vec2 b)
{
    return a.x*b.x + a.y*b.y;
}

Vec2 perp(Vec2 v)
{
    return { -v.y, v.x };
}

float perpDot(Vec2 a, Vec2 b)
{
    return dot(perp(a), b);
}

float lerp(float x, float y, float t)
{
    return (1.0f - t)*x + t*y;
}

float clamp(float x, float xMin, float xMax)
{
    float res;
    if (x < xMin)
    {
        res = xMin;
    }
    else if (x > xMax)
    {
        res = xMax;
    }
    else
    {
        res = x;
    }
    return res;
}

float getDistance(Vec2 a, Vec2 b)
{
    return getLength(a - b);
}

struct Vec3
{
    float x;
    float y;
    float z;
};

struct Mat3
{
    float m[3][3];
};

constexpr Mat3 makeI3()
{
    Mat3 m{};
    m.m[0][0] = 1.0f;
    m.m[1][1] = 1.0f;
    m.m[2][2] = 1.0f;
    return m;
}

constexpr Mat3 I3{ makeI3() };

Vec3 operator*(const Mat3& m, Vec3 v)
{
    return {
        m.m[0][0]*v.x + m.m[1][0]*v.y + m.m[2][0]*v.z,
        m.m[0][1]*v.x + m.m[1][1]*v.y + m.m[2][1]*v.z,
        m.m[0][2]*v.x + m.m[1][2]*v.y + m.m[2][2]*v.z,
    };
}

struct Mat4
{
    float m[4][4];
};

Vec2 makeVec2(Vec3 v)
{
    return {v.x, v.y};
}

struct Mat2
{
    float m[2][2];
};

Mat2 makeRotationMat2(float angle)
{
    Mat2 m;
    float c = cosf(angle);
    float s = sinf(angle);
    m.m[0][0] = c;  m.m[1][0] = -s;
    m.m[0][1] = s;  m.m[1][1] = c;
    return m;
}

Vec2 operator*(Mat2 m, Vec2 v)
{
    return {
        m.m[0][0] * v.x + m.m[1][0] * v.y,
        m.m[0][1] * v.x + m.m[1][1] * v.y,
    };
}

Vec2 makeVec2FromAngle(float angle, float len = 1.0f)
{
    return { cosf(angle) * len, sinf(angle) * len };
}

// Reflect around Y axis
Vec2 reflect(Vec2 v)
{
    return { -v.x, v.y };
}

float getAngle(Vec2 v)
{
    float a = atan2f(v.y, v.x);
    if (fabsf(a) < 0.000001f)
    {
        a = 0.0f;
    }
    else if (a < 0)
    {
        a += twoPi;
    }
    return a;
}

// Reflect around Y axis
float reflectAngle(float angle)
{
    return getAngle(reflect(makeVec2FromAngle(angle)));
}

struct Circle
{
    Vec2 p;
    float r;
};

struct DefaultVertex
{
    Vec2 pos;
    Vec3 col;
};

constexpr int numFlippers = 2;

constexpr int debugVertsCap = 64;

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
    GLuint plungerVao;

    GLuint debugVao;
    GLuint debugVbo;
};

constexpr int numCircles = 1;

struct StuffToRender
{
    Circle circles[numCircles];
    Mat3 flipperTransforms[numFlippers];

    float plungerCenterX;
    float plungerScaleY;

    DefaultVertex debugVerts[debugVertsCap];
    int numDebugVerts;
};

namespace Constants
{
    constexpr float worldSize{ 70.0f };
    constexpr float worldL{ -worldSize/2.0f };
    constexpr float worldR{ worldSize/2.0f };
    constexpr float worldT{ worldSize };
    constexpr float worldB{ 0.0f };
}

struct Arc
{
    Vec2 p;
    float r;
    float start;
    float end;
};

constexpr float maxAngularVelocity{ twoPi * 4.0f };

constexpr float radians(float deg)
{
    return pi * deg / 180.0f;
}

constexpr float leftFlipperMinAngle{ radians(-38.0f) };
constexpr float leftFlipperMaxAngle{ radians(33.0f) };

struct Flipper
{
    static constexpr float r0{ 1.1f };
    static constexpr float r1{ 0.7f };
    static constexpr float width{ 8.0f };
    static constexpr float d{ width - r0 - r1 };

    Mat3 transform;
    Vec2 position;
    float minAngle;
    float maxAngle;
    float orientation;
    float angularVelocity;
};

void updateTransform(Flipper* f)
{
    float c = cosf(f->orientation);
    float s = sinf(f->orientation);

    // T*R matrix
    f->transform.m[0][0] = c;
    f->transform.m[0][1] = s;
    f->transform.m[0][2] = 0.0f;

    f->transform.m[1][0] = -s;
    f->transform.m[1][1] = c;
    f->transform.m[1][2] = 0.0f;

    f->transform.m[2][0] = f->position.x;
    f->transform.m[2][1] = f->position.y;
    f->transform.m[2][2] = 1.0f;
}

Flipper makeFlipper(Vec2 position, bool isLeft)
{
    Flipper f;
    f.position = position;
    f.minAngle = isLeft ? leftFlipperMinAngle : reflectAngle(leftFlipperMaxAngle);
    f.maxAngle = isLeft ? leftFlipperMaxAngle : reflectAngle(leftFlipperMinAngle);
    f.orientation = isLeft ? f.minAngle : f.maxAngle;
    f.angularVelocity = 0.0f;
    updateTransform(&f);
    return f;
}

struct Ball
{
    Vec2 p;
    Vec2 v;
};

struct LineSegment
{
    Vec2 p0;
    Vec2 p1;
};

struct Line
{
    Vec2 p; // point on the line
    Vec2 d; // direction

    Line(Vec2 P, Vec2 D) : p{P}, d{D}
    {}

    Line(Vec2 P, float a) : p{P}, d{ cosf(a), sinf(a) }
    {}

    Line parallel(float offset)
    {
        Vec2 n{ perp(d) };
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
    Vec2 p;
    Vec2 d;
};

constexpr Vec3 defCol{ 1.0f, 1.0f, 1.0f };
constexpr Vec3 auxCol{ 0.5f, 0.5f, 0.5f };
constexpr Vec3 oneWayWallsColor{ 0.5f, 0.5f, 0.8f };
constexpr Vec3 highlightCol{ 0.8f, 0.0f, 0.3f };

LineSegment* addLineSegmentMirrored(LineSegment* ptr, Vec2 p0, Vec2 p1)
{
    *ptr++ = {p0, p1};
    *ptr++ = {{-p0.x, p0.y}, {-p1.x, p1.y}};
    return ptr;
}

Vec2 findIntersection(Line L1, Line L2)
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

DefaultVertex* addLine(DefaultVertex* ptr, const Line& l, Vec3 color = auxCol)
{
    constexpr float len{ 100.0f };
    *ptr++ = {l.p + l.d*len, color};
    *ptr++ = {l.p - l.d*len, color};
    return ptr;
}

DefaultVertex* addRay(DefaultVertex* ptr, const Ray& r, Vec3 color = auxCol)
{
    constexpr float len{ 100.0f };
    *ptr++ = {r.p, color};
    *ptr++ = {r.p + r.d*len, color};
    return ptr;
}

DefaultVertex* addLineStrip(DefaultVertex* ptr, Vec2* pts, int numPts)
{
    assert(numPts > 1);
    for (int i = 0; i < numPts-1; ++i)
    {
        *ptr++ = {pts[i], defCol};
        *ptr++ = {pts[i+1], defCol};
    }
    return ptr;
}

LineSegment* addLineStrip(LineSegment* ptr, Vec2* pts, int numPts, float xScale = 1.0f)
{
    assert(numPts > 1);
    for (int i = 0; i < numPts-1; ++i)
    {
        Vec2 p0 = { pts[i].x * xScale, pts[i].y };
        Vec2 p1 = { pts[i+1].x * xScale, pts[i+1].y };
        *ptr++ = { p0, p1 };
    }
    return ptr;
}

LineSegment* addLineStripMirrored(LineSegment* ptr, Vec2* pts, int numPts)
{
    ptr = addLineStrip(ptr, pts, numPts, 1.0f);
    ptr = addLineStrip(ptr, pts, numPts, -1.0f);
    return ptr;
}

DefaultVertex* addCircleLines(DefaultVertex* ptr, Vec2 p, float r)
{
    constexpr int numVerts{ 32 };
    Vec3 color{ 1.0f, 1.0f, 1.0f };

    DefaultVertex v0{
        p + Vec2{ 1.0f, 0.0f } * r,
        color,
    };

    *ptr++ = v0;

    for (int i{ 1 }; i < numVerts; ++i)
    {
        const float t{ (float)i / numVerts };
        const float angle{ t * twoPi };
        const DefaultVertex v {
            p + Vec2{ cosf(angle), sinf(angle) } * r,
            color,
        };
        *ptr++ = v;
        *ptr++ = v;
    }

    *ptr++ = v0;
    return ptr;
}

// P - intersection of two lines
// d1 - direction of the line to the left of the circle (from intersection towards circle, unit)
// d2 - direction of the line to the right of the circle (from intersection towards circle, unit)
// r - radius of the circle
// returns the position of the circle
Vec2 findCircleBetweenLines(Vec2 P, Vec2 d1, Vec2 d2, float r)
{
    Vec2 d1p = perp(d1);
    Vec2 d2p = perp(d2);
    float t = r * getLength(d1p + d2p) / getLength(d1 - d2);
    Vec2 O = P + d1*t - d1p*r;
    return O;
}

// Circular through 2 points
Arc makeArc(Vec2 pStart, Vec2 pEnd, float r)
{
    Vec2 pMid{ (pStart + pEnd) / 2.0f };
    Vec2 L{ -normalize(perp(pStart - pEnd)) };
    float m{ getLength(pMid-pEnd) };
    float l{ fabsf(r-m) < 0.001f ? 0.0f : sqrtf(r*r - m*m) };
    Vec2 c{pMid + L*l};
    float start{ getAngle(pStart - c) };
    float end{ getAngle(pEnd - c) };
    return {c, r, start, end};
}

struct ArcPoints
{
    Vec2 pStart;
    Vec2 pEnd;
};

// P - intersection of two lines
// d1 - direction of the line to the left of the circle (from intersection towards circle, unit)
// d2 - direction of the line to the right of the circle (from intersection towards circle, unit)
// r - radius of the circle
// returns the position of the circle
ArcPoints findArcBetweenLines(Vec2 P, Vec2 d1, Vec2 d2, float r)
{
    Vec2 d1p = perp(d1);
    Vec2 d2p = perp(d2);
    float t = r * getLength(d1p + d2p) / getLength(d1 - d2);
    Vec2 Q = P + d1*t;
    Vec2 R = P + d2*t;
    return {Q, R};
}

DefaultVertex* addArcLines(DefaultVertex* ptr, const Arc& arc, int numSteps = 32, Vec3 color = defCol)
{
    assert(0.0f <= arc.start && arc.start < twoPi);
    assert(0.0f <= arc.end && arc.end < twoPi);

    float start{ arc.start };
    float end{ arc.end };
    if (arc.end < arc.start)
    {
        end += twoPi;
    }

    for (int i{ 0 }; i < numSteps; ++i)
    {
        const float t{ (float)i / (numSteps - 1) };
        const float angle{ lerp(start, end, t) };
        float x = arc.p.x + cosf(angle) * arc.r;
        float y = arc.p.y + sinf(angle) * arc.r;
        DefaultVertex v = { {x, y}, color };
        *ptr++ = v;
        if (i > 0 && i < numSteps - 1)
        {
            *ptr++ = v;
        }
    }

    return ptr;
}

Arc reflectArc(const Arc& arc)
{
    Arc result;
    result.p = reflect(arc.p);
    result.r = arc.r;
    result.start = reflectAngle(arc.end);
    result.end = reflectAngle(arc.start);
    return result;
}

DefaultVertex* addSlingshotCircle(DefaultVertex* ptr, Vec2 P, Vec2 d1, Vec2 d2, float r)
{
    Vec2 O = findCircleBetweenLines(P, d1, d2, r);
    return addCircleLines(ptr, O, r);
}

// Circle through 2 points with the given radius
Circle makeCircle(Vec2 p1, Vec2 p2, float r)
{
    Vec2 p3{ (p1 + p2) / 2.0f };
    Vec2 L{ -normalize(perp(p2 - p1)) };
    float m{ getLength(p3-p1) };
    float l{ sqrtf(r*r - m*m) };
    Vec2 c{p3 + L*l};
    return {c, r};
}

Vec2 getArcStart(const Arc& arc)
{
    return arc.p + Vec2{cosf(arc.start), sinf(arc.start)} * arc.r;
}

Vec2 getArcEnd(const Arc& arc)
{
    return arc.p + Vec2{cosf(arc.end), sinf(arc.end)} * arc.r;
}

Vec2 findIntersection(const Ray& r, const Arc& a)
{
    float n = r.p.x - a.p.x;
    float m = r.p.y - a.p.y;
    float b = 2.0f*(r.d.x*n + r.d.y*m);
    float c = n*n + m*m - a.r*a.r;
    float D = b*b - 4*c;
    float t = (-b + sqrtf(D)) / 2.0f;
    return r.p + r.d * t;
}

constexpr float capsuleHalfHeight = 0.7f;
constexpr float capsuleRadius = 0.2f;

DefaultVertex* addCapsuleLines(DefaultVertex* ptr, Vec2 c)
{
    float hw=capsuleRadius;
    float hh=capsuleHalfHeight;
    Vec2 tl{c.x-hw,c.y+hh};
    Vec2 tr{c.x+hw,c.y+hh};
    Vec2 bl{c.x-hw,c.y-hh};
    Vec2 br{c.x+hw,c.y-hh};
    *ptr++ = {tl, defCol};
    *ptr++ = {bl, defCol};
    *ptr++ = {tr, defCol};
    *ptr++ = {br, defCol};
    int s = 4;
    ptr = addArcLines(ptr, makeArc(tr, tl, hw), s);
    ptr = addArcLines(ptr, makeArc(bl, br, hw), s);
    return ptr;
}

constexpr float popBumperRadius = 2.75f;

DefaultVertex* addPopBumperLines(DefaultVertex* ptr, Vec2 c)
{
    float rb{popBumperRadius};
    float gap{0.45f};
    float rs{rb-gap};
    ptr = addCircleLines(ptr, c, rb);
    ptr = addCircleLines(ptr, c, rs);
    return ptr;
}

constexpr float buttonHalfWidth = 1.4f;
constexpr float buttonHeight = 0.6f;

struct Button
{
    Vec2 p;
    Vec2 n; // normal
};

Button* addButton(Button* ptr, Vec2 p0, Vec2 p1, float t)
{
    Vec2 D{p1-p0};
    Vec2 c{ p0 + D*t };
    Vec2 d{normalize(D)};
    Vec2 dp = perp(d);
    *ptr++ = {c, dp};
    return ptr;
}

void getButtonPoints(Button b, Vec2 pts[4])
{
    Vec2 d = -perp(b.n);
    Vec2 q0 = b.p - d * buttonHalfWidth;
    Vec2 q3 = b.p + d * buttonHalfWidth;
    Vec2 q1 = q0 + b.n * buttonHeight;
    Vec2 q2 = q3 + b.n * buttonHeight;
    pts[0] = q0;
    pts[1] = q1;
    pts[2] = q2;
    pts[3] = q3;
}

DefaultVertex* addButtonLines(DefaultVertex* ptr, Button b)
{
    Vec2 pts[4];
    getButtonPoints(b, pts);
    ptr = addLineStrip(ptr, pts, 4);
    return ptr;
}

static const char* const vertexCode = R"(
#version 410

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec3 inCol;

out vec3 col;

uniform mat3 model;
uniform mat3 view;
uniform mat4 projection;

void main()
{
    col = inCol;
    gl_Position = projection * vec4(view * model * vec3(inPos, 1.0), 1.0);
}
)";

static const char* const fragmentCode = R"(
#version 410

in vec3 col;

out vec4 fragColor;

void main()
{
    fragColor = vec4(col, 1.0);
}
)";

static GLuint createShaderProgram(const char* vCode, const char* fCode)
{
    GLchar infoLog[512];
    GLint success;

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vCode, nullptr);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vs, sizeof(infoLog), nullptr, infoLog);
        fprintf(stderr, "Vertex shader error:\n%s\n", infoLog);
        exit(1);
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fCode, nullptr);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fs, sizeof(infoLog), nullptr, infoLog);
        fprintf(stderr, "Fragment shader error:\n%s\n", infoLog);
        exit(1);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        fprintf(stderr, "Program link error:\n%s\n", infoLog);
        exit(1);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

static GLuint createVao(DefaultVertex* verts, int numVerts, GLuint* vboOut = nullptr)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(verts[0]), verts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(verts[0]), (void *)offsetof(DefaultVertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(verts[0]), (void *)offsetof(DefaultVertex, col));

    if (vboOut)
    {
        *vboOut = vbo;
    }

    return vao;
}

constexpr int numFlipperCircleSegments1{ 16 };
constexpr int numFlipperCircleSegments2{ 8 };
constexpr int numFlipperVerts{ (numFlipperCircleSegments1 + 1) + (numFlipperCircleSegments2 + 1) };

static void makeFlipperVerts(DefaultVertex* verts)
{
    constexpr float cosA{ (Flipper::r0 - Flipper::r1) / Flipper::d };
    const float a{ acosf(cosA) };
    constexpr Vec3 color{ 1.0f, 1.0f, 1.0f };

    int nextVert{ 0 };

    for (int i{ 0 }; i <= numFlipperCircleSegments1; ++i)
    {
        const float t{ (float)i / numFlipperCircleSegments1 };
        const float angle{ a + 2.0f * t * (pi - a) };
        const float x{ Flipper::r0 * cosf(angle) };
        const float y{ Flipper::r0 * sinf(angle) };
        verts[nextVert++] = { { x, y }, color };
    }

    for (int i{ 0 }; i <= numFlipperCircleSegments2; ++i)
    {
        const float t{ (float)i / numFlipperCircleSegments2 };
        const float angle{ -a + t * 2.0f * a };
        const float x{ Flipper::d + Flipper::r1 * cosf(angle) };
        const float y{ Flipper::r1 * sinf(angle) };
        verts[nextVert++] = { { x, y }, color };
    }

    assert(nextVert == numFlipperVerts);
}

constexpr int numCircleVerts{ 64 };

static void makeCircleVerts(DefaultVertex* verts)
{
    for (int i{ 0 }; i < numCircleVerts; ++i)
    {
        const float t{ (float)i / numCircleVerts };
        const float angle{ t * twoPi };
        verts[i] = {
            { cosf(angle), sinf(angle) },
            { 1.0f, 1.0f, 1.0f },
        };
    }
}

constexpr int plungerNumSections{ 10 };
constexpr int numPlungerVerts{ plungerNumSections + 2 };

static void makePlungerVerts(DefaultVertex* verts)
{
    constexpr float halfWidth{ 1.0f };
    int n{ 0 };
    verts[n++] = { {halfWidth, 1.0f}, defCol };
    verts[n++] = { {-halfWidth, 1.0f}, defCol };
    for (int i{ 1 }; i <= plungerNumSections; ++i)
    {
        const float x{ (i % 2 == 0) ? -halfWidth : halfWidth };
        const float y{ 1.0f - (1.0f / plungerNumSections) * i };
        verts[n++] = { {x, y}, defCol };
    }
    assert(n == numPlungerVerts);
}

static RenderData g_renderData;
static StuffToRender g_stuffToRender;

static void render(RenderData* rd, StuffToRender* s)
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw moving circles
    for (int i = 0; i < numCircles; ++i)
    {
        Circle c = s->circles[i];

        float m[9];
        m[0] = c.r;   m[3] = 0.0f;  m[6] = c.p.x;
        m[1] = 0.0f;  m[4] = c.r;   m[7] = c.p.y;
        m[2] = 0.0f;  m[5] = 0.0f;  m[8] = 1.0f;
        glUniformMatrix3fv(rd->modelLoc, 1, GL_FALSE, m);

        glBindVertexArray(rd->circleVao);
        glDrawArrays(GL_LINE_LOOP, 0, numCircleVerts);
    }

    // Draw flippers
    for (int i = 0; i < numFlippers; ++i)
    {
        glUniformMatrix3fv(rd->modelLoc, 1, GL_FALSE, &s->flipperTransforms[i].m[0][0]);
        glBindVertexArray(rd->flipperVao);
        glDrawArrays(GL_LINE_LOOP, 0, numFlipperVerts);
    }

    // Draw static lines
    glUniformMatrix3fv(rd->modelLoc, 1, GL_FALSE, &I3.m[0][0]);
    glBindVertexArray(rd->lineVao);
    glDrawArrays(GL_LINES, 0, rd->numLineVerts);

    // Draw the plunger
    {
        float m[9];
        m[0] = 1.0f;  m[3] = 0.0f;              m[6] = s->plungerCenterX;
        m[1] = 0.0f;  m[4] = s->plungerScaleY;  m[7] = 0.0f;
        m[2] = 0.0f;  m[5] = 0.0f;              m[8] = 1.0f;
        glBindVertexArray(rd->plungerVao);
        glUniformMatrix3fv(rd->modelLoc, 1, GL_FALSE, m);
        glDrawArrays(GL_LINE_STRIP, 0, numPlungerVerts);
    }

    // Draw debug lines
    {
        glBindVertexArray(rd->debugVao);
        glBindBuffer(GL_ARRAY_BUFFER, rd->debugVbo);
        glBufferData(GL_ARRAY_BUFFER, s->numDebugVerts * sizeof(s->debugVerts[0]), s->debugVerts, GL_DYNAMIC_DRAW);
        glUniformMatrix3fv(rd->modelLoc, 1, GL_FALSE, &I3.m[0][0]);
        glDrawArrays(GL_LINES, 0, s->numDebugVerts);
    }
}

void drawDebugLine(Vec2 p0, Vec2 p1)
{
    auto& s = g_stuffToRender;
    assert(s.numDebugVerts+1 < debugVertsCap);
    s.debugVerts[s.numDebugVerts++] = { p0, highlightCol };
    s.debugVerts[s.numDebugVerts++] = { p1, highlightCol };
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

    fprintf(stderr, "\nOpenGL debug message (%u): %s\n", id, message);

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             fprintf(stderr, "Source: API\n"); break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   fprintf(stderr, "Source: Window System\n"); break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: fprintf(stderr, "Source: Shader Compiler\n"); break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     fprintf(stderr, "Source: Third Party\n"); break;
    case GL_DEBUG_SOURCE_APPLICATION:     fprintf(stderr, "Source: Application\n"); break;
    case GL_DEBUG_SOURCE_OTHER:           fprintf(stderr, "Source: Other\n"); break;
    default:                              fprintf(stderr, "Source: ???\n"); break;
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               fprintf(stderr, "Type: Error\n"); break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: fprintf(stderr, "Type: Deprecated Behaviour\n"); break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  fprintf(stderr, "Type: Undefined Behaviour\n"); break;
    case GL_DEBUG_TYPE_PORTABILITY:         fprintf(stderr, "Type: Portability\n"); break;
    case GL_DEBUG_TYPE_PERFORMANCE:         fprintf(stderr, "Type: Performance\n"); break;
    case GL_DEBUG_TYPE_MARKER:              fprintf(stderr, "Type: Marker\n"); break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          fprintf(stderr, "Type: Push Group\n"); break;
    case GL_DEBUG_TYPE_POP_GROUP:           fprintf(stderr, "Type: Pop Group\n"); break;
    case GL_DEBUG_TYPE_OTHER:               fprintf(stderr, "Type: Other\n"); break;
    default:                                fprintf(stderr, "Type: ???\n"); break;
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         fprintf(stderr, "Severity: high\n"); break;
    case GL_DEBUG_SEVERITY_MEDIUM:       fprintf(stderr, "Severity: medium\n"); break;
    case GL_DEBUG_SEVERITY_LOW:          fprintf(stderr, "Severity: low\n"); break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: fprintf(stderr, "Severity: notification\n"); break;
    default:                             fprintf(stderr, "Severity: ???\n"); break;
    }
}

static void errorCallback(int /*error*/, const char* description)
{
    fprintf(stderr, "GLFW error: %s\n", description);
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
    render(&g_renderData, &g_stuffToRender);
    glfwSwapBuffers(window);
}

Mat4 myOrtho(float l, float r, float b, float t, float n, float f)
{
    Mat4 m{};
    m.m[0][0] = 2.0f/(r-l);
    m.m[1][1] = 2.0f/(t-b);
    m.m[2][2] = -2.0f/(f-n);
    m.m[3][0] = -(r+l)/(r-l);
    m.m[3][1] = -(t+b)/(t-b);
    m.m[3][2] = -(f+n)/(f-n);
    m.m[3][3] = 1.0f;
    return m;
}

float getRandomFloat(float min, float max)
{
    return min + (max - min) * rand() / RAND_MAX;
}

void resolveCollision(Ball* ball, Vec2 normal, float penetration, float relativeNormalVelocity, float bounciness = 0.5f)
{
    if (relativeNormalVelocity <= 0.0f)
    {
        ball->p += normal * penetration;

        if (bounciness > 1.0f)
        {
            // Add random offset to the normal
            constexpr float delta = radians(5.0f);
            float angle = getRandomFloat(-delta, delta);
            Mat2 rotation = makeRotationMat2(angle);
            normal = rotation * normal;
        }

        Vec2 tangent = perp(normal);

        float initNormalSpeed = dot(ball->v, normal);
        float initTangentSpeed = dot(ball->v, tangent);

        constexpr float friction = 0.99f;

        float targetNormalSpeed = initNormalSpeed - (1.0f + bounciness) * relativeNormalVelocity;
        float targetTangentSpeed = initTangentSpeed * friction;

        ball->v = normal * targetNormalSpeed + tangent * targetTangentSpeed;
    }
}

struct Collision
{
    Vec2 normal;
    float penetration;
};

Collision checkIntersection(const Circle& circ, const Arc& arc)
{
    Vec2 v{ normalize(circ.p - arc.p) * arc.r };

    float a{ atan2f(v.y, v.x) };
    if (a < 0.0f)
    {
        a += twoPi;
    }

    float b{ a - arc.start };
    if (b < 0.0f) b += twoPi;
    float end{ arc.end - arc.start };
    if (end < 0.0f) end += twoPi;

    float closestAngle{};
    if (b < end)
    {
        closestAngle = a;
    }
    else
    {
        if ((twoPi - b) < (b - end))
        {
            closestAngle = arc.start;
        }
        else
        {
            closestAngle = arc.end;
        }
    }

    Vec2 w{ cosf(closestAngle), sinf(closestAngle) };
    Vec2 closestPoint{ arc.p + w * arc.r };
    Vec2 vv{ circ.p - closestPoint };
    Vec2 normal{ normalize(vv) };
    float penetration{ circ.r - getLength(vv) };

    return {
        normal,
        penetration,
    };
}

int main()
{
    srand((unsigned int)time(NULL));

    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    GLFWwindow* window{ glfwCreateWindow(800, 800, "my_pinball", nullptr, nullptr) };
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return 1;
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

    constexpr float flipperX{ 8.0f };
    constexpr float flipperY{ 7.0f };

    RenderData* renderData = &g_renderData;
    StuffToRender* stuffToRender = &g_stuffToRender;

    constexpr int basicWallsCap = 70;
    LineSegment basicWalls[basicWallsCap];
    int numBasicWalls;

    constexpr int slingshotWallsCap = 2;
    LineSegment slingshotWalls[slingshotWallsCap];
    int numSlingshotWalls;

    constexpr int oneWayWallsCap = 2;
    LineSegment oneWayWalls[oneWayWallsCap];
    int numOneWayWalls;

    constexpr int arcsCap = 16;
    Arc arcs[arcsCap];
    int arcSteps[arcsCap];
    int numArcs = 0;

#define ADD_ARC(arc, steps)    \
    arcs[numArcs] = arc;       \
    arcSteps[numArcs] = steps; \
    ++numArcs;

#define ADD_ARC_MIRRORED(arc, steps) \
    ADD_ARC(arc, steps);             \
    ADD_ARC(reflectArc(arc), steps);

    constexpr int capsulesCap = 2;
    Vec2 capsules[capsulesCap];
    int numCapsules;

    constexpr int popBumpersCap = 3;
    Vec2 popBumpers[popBumpersCap];
    int numPopBumpers;

    constexpr int buttonsCap = 16;
    Button buttons[buttonsCap];
    int numButtons;

    float plungerCenterX;
    float plungerTopY;

    {
        LineSegment* basicWallsPtr = basicWalls;
        LineSegment* slingshotWallsPtr = slingshotWalls;
        LineSegment* oneWayWallsPtr = oneWayWalls;
        Vec2* capsulesPtr = capsules;
        Vec2* popBumpersPtr = popBumpers;
        Button* buttonsPtr = buttons;

        const Vec2 p0{ -flipperX - 0.5f, flipperY + Flipper::r0 + 0.5f };

        Line l0{ p0, leftFlipperMinAngle };
        Line l1{ Line::vertical(-flipperX - 9.0f) };

        const Vec2 p1{ findIntersection(l0, l1) };
        const Vec2 p2{ p1 + Vec2{ 0.0f, 14.0f } };

        // Angled wall right near the flipper
        Vec2 strip1[] = { p0,p1,p2 };
        basicWallsPtr = addLineStrip(basicWallsPtr, strip1, ARRAY_LEN(strip1));

        Vec2 p0r = reflect(p0);
        Vec2 p1r = reflect(p1);
        Vec2 p2r = p1r + Vec2{ 0.0f, 16.0f };
        Vec2 strip1r[] = { p0r,p1r,p2r };
        basicWallsPtr = addLineStrip(basicWallsPtr, strip1r, ARRAY_LEN(strip1r));

        Line l2{ l0.parallel(-5.0f) };
        Line l3{ l1.parallel(4.0f) };
        Line l4{ Line::vertical(-flipperX - 4.5f) };

        Line worldB{ Line::horizontal(Constants::worldB) };

        // ditch
        Vec2 pp1 = findIntersection(l1, l2);
        Line ll1 = Line::horizontal(pp1.y - 3.0f);
        Vec2 pp2 = findIntersection(l1, ll1);
        Vec2 pp3 = findIntersection(ll1, l3);

        const Vec2 p3{ findIntersection(l4, worldB) };
        const Vec2 p4{ findIntersection(l2, l4) };
        const Vec2 p6{ pp3.x, pp3.y + 20.0f };

        // Outer wall near the flipper
        Vec2 strip2[] = { p3,p4,pp1,pp2,pp3 };
        basicWallsPtr = addLineStripMirrored(basicWallsPtr, strip2, ARRAY_LEN(strip2));
        *basicWallsPtr++ = {pp3, p6};

        // vertical wall near the right flipper
        Vec2 pp3r = { -pp3.x, pp3.y };
        Vec2 p8 = { pp3r.x, pp3r.y + 23.6f };
        *basicWallsPtr++ = {pp3r, p8};

        Vec2 p80 = findIntersection(l2, l3);
        // ditch caps
        basicWallsPtr = addLineSegmentMirrored(basicWallsPtr, pp1, p80);

        // Construct slingshot
        {
            Line sL{ l1.parallel(-3.0f) };
            Line sB{ l0.parallel(3.5f) };
            Vec2 sLB{ findIntersection(sL, sB) };
            Line sLB1{ sLB, radians(109.0f) };
            Line sR{ sLB1.parallel(-4.0f) };
            Vec2 sRB{ findIntersection(sR, sB) };
            Vec2 sLR{ findIntersection(sL, sR) };

            float LRr = 0.8f;
            ArcPoints apLR = findArcBetweenLines(sLR, -sR.d, -sL.d, LRr);
            Arc arcLR = makeArc(apLR.pStart, apLR.pEnd, LRr);
            ADD_ARC_MIRRORED(arcLR, 8);

            float RBr = 0.82f;
            ArcPoints apRB = findArcBetweenLines(sRB, -sB.d, sR.d, RBr);
            Arc arcRB = makeArc(apRB.pStart, apRB.pEnd, RBr);
            ADD_ARC_MIRRORED(arcRB, 8);

            float LBr = 2.0f;
            ArcPoints apLB = findArcBetweenLines(sLB, sL.d, sB.d, LBr);
            Arc arcLB = makeArc(apLB.pStart, apLB.pEnd, LBr);
            ADD_ARC_MIRRORED(arcLB, 8);

            slingshotWallsPtr = addLineSegmentMirrored(slingshotWallsPtr, apLR.pStart, apRB.pEnd);
            basicWallsPtr = addLineSegmentMirrored(basicWallsPtr, apRB.pStart, apLB.pEnd);
            basicWallsPtr = addLineSegmentMirrored(basicWallsPtr, apLB.pStart, apLR.pEnd);
        }

        Vec2 p7{ p2 + Vec2{2.0f, 7.0f} };

        // left bottom arc
        ADD_ARC(makeArc(p7, p6, 10.0f), 8);

        Vec2 p9 = { p8.x - 7.5f,p8.y + 10.0f };
        ADD_ARC(makeArc(p8, p9, 11.0f), 8);

        Line l3r{ {-l3.p.x,l3.p.y}, l3.d };
        Line l20 = l3r.parallel(-0.5f);
        float plungerShuteWidth = 3.4f;
        Line l21 = l20.parallel(-plungerShuteWidth);

        Vec2 p20 = findIntersection(l20, worldB);
        Vec2 p21 = findIntersection(l21, worldB);
        float k20 = 48.0f;
        Vec2 p22 = p20 + Vec2{ 0.0f, 1.0f } *k20;
        Vec2 p23 = p21 + Vec2{ 0.0f, 1.0f } *k20;
        // Plunger shaft
        *basicWallsPtr++ = {p20, p22};
        *basicWallsPtr++ = {p21, p23};

        // Top of the plunger
        Vec2 p30 = findIntersection(ll1, l20);
        Vec2 p31 = findIntersection(ll1, l21);
        *basicWallsPtr++ = {p30, p31};
        plungerCenterX = ((p30 + p31) / 2.0f).x;
        plungerTopY = p30.y;

        float arc30r = 20.87f;
        Vec2 arc30c = p23 + Vec2{ -arc30r, 0.0f };
        Arc arc30{ arc30c, arc30r, 0.0f, radians(90.0f) };
        ADD_ARC(arc30, 16);

        float arc31r = 20.87f - plungerShuteWidth;
        Arc arc31{ arc30c, arc31r, 0.0f, radians(84.0f) };
        ADD_ARC(arc31, 16);

        // Right upper wall
        Vec2 p10 = p9 + makeVec2FromAngle(radians(110.0f), 4.5f);
        Vec2 p11 = p10 + makeVec2FromAngle(radians(31.0f), 5.3f);
        Vec2 p12 = p11 + makeVec2FromAngle(radians(97.0f), 12.2f);
        Vec2 p13 = p12 + makeVec2FromAngle(radians(150.0f), 10.85f);
        Vec2 p14 = getArcEnd(arc31);
        Vec2 strip3[] = { p9,p10,p11,p12,p13,p14 };
        basicWallsPtr = addLineStrip(basicWallsPtr, strip3, ARRAY_LEN(strip3));

        buttonsPtr = addButton(buttonsPtr, p9, p10, 0.5f);
        buttonsPtr = addButton(buttonsPtr, p10, p11, 0.5f);
        buttonsPtr = addButton(buttonsPtr, p11, p12, 0.3f);
        buttonsPtr = addButton(buttonsPtr, p11, p12, 0.7f);
        buttonsPtr = addButton(buttonsPtr, p12, p13, 0.5f);

        Ray r30{ p14, normalize(p14 - p13) };
        Vec2 p15 = findIntersection(r30, arc30);
        // right one-way wall
        *oneWayWallsPtr++ = { p14,p15 };

        Vec2 p40 = getArcEnd(arc30);
        Vec2 p41 = p40 + Vec2{ -7.68f, 0.0f };
        // bridge between left and right arcs at the top of the table
        *basicWallsPtr++ = {p40, p41};

        // left top big arc
        Arc a50 = makeArc(p41, p7, 20.8f);
        ADD_ARC(a50, 16);

        // left small arc
        Arc a51 = { a50.p, a50.r - plungerShuteWidth, radians(105.0f), radians(130.0f) };
        ADD_ARC(a51, 16);

        // left medium arc
        Arc a52 = { a50.p, a50.r - plungerShuteWidth, radians(150.0f), radians(205.0f) };
        ADD_ARC(a52, 16);

        Vec2 a51s = getArcStart(a51);
        Ray r51s{ a51.p, normalize(a51s - a51.p) };

        Vec2 a51e = getArcEnd(a51);
        Ray r51e{ a51.p, normalize(a51e - a51.p) };

        Vec2 p50 = findIntersection(r51s, a50);
        // left one-way wall
        *oneWayWallsPtr++ = { p50, a51s };

        float w51 = 2.3f;
        Vec2 p53 = a51s - r51s.d * w51;
        Vec2 p54 = a51e - r51e.d * w51;

        // left-top walled island
        Vec2 strip4[] = { a51s,p53,p54,a51e };
        basicWallsPtr = addLineStrip(basicWallsPtr, strip4, ARRAY_LEN(strip4));
        buttonsPtr = addButton(buttonsPtr, p53, p54, 0.5f);

        Vec2 a52s = getArcStart(a52);
        Vec2 a52e = getArcEnd(a52);
        Vec2 p60 = a52e + makeVec2FromAngle(radians(-32.5f), 3.6f);
        Vec2 p61 = p60 + makeVec2FromAngle(radians(44.0f), 4.5f);
        Vec2 p62 = p61 + makeVec2FromAngle(radians(167.6f), 4.3f);
        // left-middle walled island
        Vec2 strip5[] = { a52e,p60,p61,p62,a52s };
        basicWallsPtr = addLineStrip(basicWallsPtr, strip5, ARRAY_LEN(strip5));
        buttonsPtr = addButton(buttonsPtr, p61, p60, 0.5f);
        buttonsPtr = addButton(buttonsPtr, p62, p61, 0.5f);
        buttonsPtr = addButton(buttonsPtr, a52s, p62, 0.3f);
        buttonsPtr = addButton(buttonsPtr, a52s, p62, 0.7f);

        float capsuleGap = 3.0f;
        float leftCapsuleX = 0.0f;
        float rightCapsuleX = leftCapsuleX + capsuleGap;
        float capsuleY = p53.y;
        *capsulesPtr++ = { leftCapsuleX, capsuleY };
        *capsulesPtr++ = { rightCapsuleX, capsuleY };

        Vec2 pb1{ -4.0f, 53.0f };
        Vec2 pb2{ pb1.x + 10.7f, pb1.y + 0.5f };
        Vec2 pb3{ pb1.x + 5.5f, pb1.y - 7.5f };
        *popBumpersPtr++ = pb1;
        *popBumpersPtr++ = pb2;
        *popBumpersPtr++ = pb3;

        numBasicWalls = (int)(basicWallsPtr - basicWalls);
        numSlingshotWalls = (int)(slingshotWallsPtr - slingshotWalls);
        numOneWayWalls = (int)(oneWayWallsPtr - oneWayWalls);
        numCapsules = (int)(capsulesPtr - capsules);
        numPopBumpers = (int)(popBumpersPtr - popBumpers);
        numButtons = (int)(buttonsPtr - buttons);

        assert(numBasicWalls <= basicWallsCap);
        assert(numSlingshotWalls <= slingshotWallsCap);
        assert(numOneWayWalls <= oneWayWallsCap);
        assert(numArcs <= arcsCap);
        assert(numCapsules <= capsulesCap);
        assert(numPopBumpers <= popBumpersCap);
        assert(numButtons <= buttonsCap);
    }

#undef ADD_ARC_MIRRORED
#undef ADD_ARC

    stuffToRender->plungerCenterX = plungerCenterX;

    Vec2 initialBallPosition = { plungerCenterX, plungerTopY + 3.0f };

    Ball ball;
    ball.p = initialBallPosition;
    ball.v = {};

    Flipper flippers[numFlippers];
    flippers[0] = makeFlipper(Vec2{ -flipperX, flipperY }, true);
    flippers[1] = makeFlipper(Vec2{  flipperX, flipperY }, false);

    float plungerT = 0.0f;
    constexpr float plungerDownSpeed = 1.0f;

    // Initialize render data
    {
        renderData->program = createShaderProgram(vertexCode, fragmentCode);

        renderData->modelLoc = glGetUniformLocation(renderData->program, "model");
        renderData->viewLoc = glGetUniformLocation(renderData->program, "view");
        renderData->projectionLoc = glGetUniformLocation(renderData->program, "projection");

        assert(renderData->modelLoc >= 0);
        assert(renderData->viewLoc >= 0);
        assert(renderData->projectionLoc >= 0);

        constexpr int lineVertsCap = 900;
        DefaultVertex lineVerts[lineVertsCap];
        int numLineVerts;

        // Fill lineVerts
        {
            DefaultVertex* ptr = lineVerts;

            for (int i = 0; i < numBasicWalls; ++i)
            {
                *ptr++ = { basicWalls[i].p0, defCol };
                *ptr++ = { basicWalls[i].p1, defCol };
            }

            for (int i = 0; i < numSlingshotWalls; ++i)
            {
                *ptr++ = { slingshotWalls[i].p0, defCol };
                *ptr++ = { slingshotWalls[i].p1, defCol };
            }

            for (int i = 0; i < numOneWayWalls; ++i)
            {
                *ptr++ = { oneWayWalls[i].p0, oneWayWallsColor };
                *ptr++ = { oneWayWalls[i].p1, oneWayWallsColor };
            }

            for (int i = 0; i < numArcs; ++i)
            {
                ptr = addArcLines(ptr, arcs[i], arcSteps[i]);
            }

            for (int i = 0; i < numCapsules; ++i)
            {
                ptr = addCapsuleLines(ptr, capsules[i]);
            }

            for (int i = 0; i < numPopBumpers; ++i)
            {
                ptr = addPopBumperLines(ptr, popBumpers[i]);
            }

            for (int i = 0; i < numButtons; ++i)
            {
                ptr = addButtonLines(ptr, buttons[i]);
            }

            numLineVerts = (int)(ptr - lineVerts);
            assert(numLineVerts <= lineVertsCap);
        }

        renderData->lineVao = createVao(lineVerts, numLineVerts);
        renderData->numLineVerts = numLineVerts;

        DefaultVertex circleVerts[numCircleVerts];
        makeCircleVerts(circleVerts);
        renderData->circleVao = createVao(circleVerts, numCircleVerts);

        DefaultVertex flipperVerts[numFlipperVerts];
        makeFlipperVerts(flipperVerts);
        renderData->flipperVao = createVao(flipperVerts, numFlipperVerts);

        DefaultVertex plungerVerts[numPlungerVerts];
        makePlungerVerts(plungerVerts);
        renderData->plungerVao = createVao(plungerVerts, numPlungerVerts);

        renderData->debugVao = createVao(nullptr, debugVertsCap, &renderData->debugVbo);
    }

    Mat4 projection{ myOrtho(Constants::worldL, Constants::worldR, Constants::worldB, Constants::worldT, -1.0f, 1.0f) };

    glUseProgram(renderData->program);
    glUniformMatrix3fv(renderData->viewLoc, 1, GL_FALSE, &I3.m[0][0]);
    glUniformMatrix4fv(renderData->projectionLoc, 1, GL_FALSE, &projection.m[0][0]);

    float accum = 0.0f;
    float prevTime{ (float)glfwGetTime() };

    while (!glfwWindowShouldClose(window))
    {
        float currentTime{ (float)glfwGetTime() };
        float frameDt = currentTime - prevTime;
        if (frameDt > maxDt)
        {
            frameDt = maxDt;
        }
        prevTime = currentTime;
        accum += frameDt;

        stuffToRender->numDebugVerts = 0;

        //
        // Handle input
        //

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            flippers[0].angularVelocity = maxAngularVelocity;
        }
        else
        {
            flippers[0].angularVelocity = -maxAngularVelocity;
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            flippers[1].angularVelocity = -maxAngularVelocity;
        }
        else
        {
            flippers[1].angularVelocity = maxAngularVelocity;
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            plungerT += plungerDownSpeed * frameDt;
            if (plungerT > 1.0f)
            {
                plungerT = 1.0f;
            }
        }
        else
        {
            constexpr float plungerImpulse = 300.0f;
            bool ballIsOnTopOfPlunger = fabsf((ball.p.y - 1.0f) - plungerTopY) < 0.5f;
            if (ballIsOnTopOfPlunger)
            {
                // Launch the ball
                ball.v.y += plungerImpulse * plungerT * getRandomFloat(0.8f, 1.2f);
            }
            plungerT = 0.0f;
        }

        //
        // Fixed-step physics simulation
        //

        while (accum >= simDt)
        {
            accum -= simDt;

            // Update ball
            {
                constexpr Vec2 gravity = { 0.0f, -60.0f };
                ball.v += gravity * simDt;

                const float maxSpeed{ ballRadius * simFps * 0.99f };
                if (getLength(ball.v) > maxSpeed)
                {
                    ball.v = normalize(ball.v) * maxSpeed;
                }

                ball.p += ball.v * simDt;

                if (ball.p.y + ballRadius < 0.0f)
                {
                    ball.p = initialBallPosition;
                    ball.v = {};
                }
            }

            // Update flippers
            for (int i = 0; i < numFlippers; ++i)
            {
                Flipper* f = &flippers[i];
                f->orientation = clamp(f->orientation + f->angularVelocity * simDt, f->minAngle, f->maxAngle);
                if (f->orientation == f->minAngle || f->orientation == f->maxAngle)
                {
                    f->angularVelocity = 0.0f;
                }
                updateTransform(f);
            }

            // Check collision of ball and flippers
            for (int i{ 0 }; i < numFlippers; ++i)
            {
                Flipper* flipper{ &flippers[i] };
                Vec2 p0{ makeVec2(flipper->transform * Vec3{0.0f, 0.0f, 1.0f}) };
                Vec2 p1{ makeVec2(flipper->transform * Vec3{Flipper::d, 0.0f, 1.0f}) };
                Vec2 line{ p1 - p0 };
                Vec2 lineDir{ normalize(line) };
                float t{ clamp(dot(ball.p - p0, lineDir) / getLength(line), 0.0f, 1.0f) };
                float r{ lerp(Flipper::r0, Flipper::r1, t) };
                Vec2 closestPoint{ p0 + line * t };
                float dist{ getDistance(closestPoint, ball.p) };
                float penetration = (r + ballRadius) - dist;
                Vec2 normal = normalize(ball.p - closestPoint);
                if (penetration >= 0.0f)
                {
                    Vec2 pointOnFlipperWorld{ ball.p - normal * (ballRadius - penetration) };
                    Vec2 pointOnFlipperLocal{ pointOnFlipperWorld - flipper->position };
                    Vec2 pointOnFlipperVelocity{ flipper->angularVelocity * perp(pointOnFlipperLocal) };
                    Vec2 relativeVelocity{ ball.v - pointOnFlipperVelocity };
                    float relativeNormalVelocity{ dot(relativeVelocity, normal) };
                    resolveCollision(&ball, normal, penetration, relativeNormalVelocity);
                }
            }

            // Check collisions of ball and basic walls
            for (int i = 0; i < numBasicWalls; ++i)
            {
                Vec2 p0 = basicWalls[i].p0;
                Vec2 p1 = basicWalls[i].p1;
                Vec2 L = p1 - p0;
                float segmentLength = getLength(L);
                Vec2 dir = L / segmentLength;
                float t = clamp(dot(ball.p - p0, dir), 0.0f, segmentLength);
                Vec2 closestPoint = p0 + t * dir;
                float dist = getDistance(ball.p, closestPoint);
                float penetration = ballRadius - dist;
                if (penetration >= 0.0f)
                {
                    Vec2 normal = normalize(ball.p - closestPoint);
                    Vec2 relativeVelocity = ball.v; // line segment is stationary
                    float relativeNormalVelocity = dot(relativeVelocity, normal);
                    resolveCollision(&ball, normal, penetration, relativeNormalVelocity);
                }
            }

            constexpr float popBumperBounciness = 5.0f;
            constexpr float slingshotBounciness = 4.0f;
            constexpr float buttonBounciness = 4.0f;

            // Check collisions of ball and slingshot walls
            for (int i = 0; i < numSlingshotWalls; ++i)
            {
                Vec2 p0 = slingshotWalls[i].p0;
                Vec2 p1 = slingshotWalls[i].p1;
                Vec2 L = p1 - p0;
                float segmentLength = getLength(L);
                Vec2 dir = L / segmentLength;
                float t = clamp(dot(ball.p - p0, dir), 0.0f, segmentLength);
                Vec2 closestPoint = p0 + t * dir;
                float dist = getDistance(ball.p, closestPoint);
                float penetration = ballRadius - dist;
                if (penetration >= 0.0f)
                {
                    Vec2 normal = normalize(ball.p - closestPoint);
                    Vec2 relativeVelocity = ball.v; // line segment is stationary
                    float relativeNormalVelocity = dot(relativeVelocity, normal);
                    resolveCollision(&ball, normal, penetration, relativeNormalVelocity, slingshotBounciness);
                }
            }

            // Check collisions of ball and one-way walls
            for (int i = 0; i < numOneWayWalls; ++i)
            {
                Vec2 p0 = oneWayWalls[i].p0;
                Vec2 p1 = oneWayWalls[i].p1;
                Vec2 L = p1 - p0;
                float segmentLength = getLength(L);
                Vec2 dir = L / segmentLength;
                float t = clamp(dot(ball.p - p0, dir), 0.0f, segmentLength);
                Vec2 closestPoint = p0 + t * dir;
                float dist = getDistance(ball.p, closestPoint);
                float penetration = ballRadius - dist;
                bool ballIsOnCollidinSide = perpDot(L, ball.p - p0) >= 0.0f;
                if (penetration >= 0.0f && ballIsOnCollidinSide)
                {
                    Vec2 normal = normalize(ball.p - closestPoint);
                    Vec2 relativeVelocity = ball.v; // line segment is stationary
                    float relativeNormalVelocity = dot(relativeVelocity, normal);
                    resolveCollision(&ball, normal, penetration, relativeNormalVelocity);
                }
            }

            // Check collisions of ball and arcs
            for (int i = 0; i < numArcs; ++i)
            {
                Circle circ{ ball.p, ballRadius };
                Collision c{ checkIntersection(circ, arcs[i])};
                if (c.penetration >= 0.0f)
                {
                    Vec2 relativeVelocity{ ball.v };
                    float relativeNormalVelocity{ dot(relativeVelocity, c.normal) };
                    resolveCollision(&ball, c.normal, c.penetration, relativeNormalVelocity);
                }
            }

            // Check collisions of ball and capsules
            for (int i = 0; i < numCapsules; ++i)
            {
                Vec2 capsuleCenter = capsules[i];
                Vec2 hh = { 0.0f, capsuleHalfHeight };
                Vec2 p0{ capsuleCenter - hh };
                Vec2 p1{ capsuleCenter + hh };
                Vec2 line{ p1 - p0 };
                Vec2 lineDir{ normalize(line) };
                float t{ clamp(dot(ball.p - p0, lineDir) / getLength(line), 0.0f, 1.0f) };
                Vec2 closestPoint{ p0 + line * t };
                float dist{ getDistance(closestPoint, ball.p) };
                float penetration = (capsuleRadius + ballRadius) - dist;
                Vec2 normal = normalize(ball.p - closestPoint);
                if (penetration >= 0.0f)
                {
                    Vec2 relativeVelocity{ ball.v };
                    float relativeNormalVelocity{ dot(relativeVelocity, normal) };
                    resolveCollision(&ball, normal, penetration, relativeNormalVelocity);
                }
            }

            // Check collisions of ball and pop bumpers
            for (int i = 0; i < numPopBumpers; ++i)
            {
                float dist{ getDistance(ball.p, popBumpers[i]) };
                float penetration = (ballRadius + popBumperRadius) - dist;
                Vec2 normal = normalize(ball.p - popBumpers[i]);
                if (penetration >= 0.0f)
                {
                    Vec2 relativeVelocity{ ball.v };
                    float relativeNormalVelocity{ dot(relativeVelocity, normal) };
                    resolveCollision(&ball, normal, penetration, relativeNormalVelocity, popBumperBounciness);
                }
            }

            // Check collisions of ball and buttons
            for (int i = 0; i < numButtons; ++i)
            {
                Vec2 pts[4];
                getButtonPoints(buttons[i], pts);
                Vec2 p0 = pts[1];
                Vec2 p1 = pts[2];
                Vec2 L = p1 - p0;
                float segmentLength = getLength(L);
                Vec2 dir = L / segmentLength;
                float t = clamp(dot(ball.p - p0, dir), 0.0f, segmentLength);
                Vec2 closestPoint = p0 + t * dir;
                float dist = getDistance(ball.p, closestPoint);
                float penetration = ballRadius - dist;
                if (penetration >= 0.0f)
                {
                    Vec2 normal = buttons[i].n;
                    Vec2 relativeVelocity = ball.v; // line segment is stationary
                    float relativeNormalVelocity = dot(relativeVelocity, normal);
                    resolveCollision(&ball, normal, penetration, relativeNormalVelocity, buttonBounciness);
                }
            }
        }

        //
        // Render the frame
        //

        stuffToRender->circles[0] = {ball.p, ballRadius};

        for (int i = 0; i < numFlippers; ++i)
        {
            stuffToRender->flipperTransforms[i] = flippers[i].transform;
        }

        stuffToRender->plungerScaleY = plungerTopY * (1.0f - plungerT);

        render(renderData, stuffToRender);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}
