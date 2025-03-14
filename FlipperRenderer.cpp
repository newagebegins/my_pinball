#include "FlipperRenderer.h"

static constexpr int numCircleSegments1{ 16 };
static constexpr int numCircleSegments2{ 8 };
static constexpr int numVerts{ (numCircleSegments1 + 1) + (numCircleSegments2 + 1) };

static std::vector<DefaultVertex> makeVerts()
{
    constexpr float width{ 8.0f };
    constexpr float d{ width - Flipper::r0 - Flipper::r1 };
    constexpr float cosA{ (Flipper::r0 - Flipper::r1) / d };
    const float a{ std::acos(cosA) };
    constexpr glm::vec3 color{ 1.0f, 1.0f, 1.0f };

    std::vector<DefaultVertex> verts(numVerts);

    std::size_t nextVert{ 0 };

    for (int i{ 0 }; i <= numCircleSegments1; ++i)
    {
        const float t{ static_cast<float>(i) / numCircleSegments1 };
        const float angle{ a + 2.0f * t * (glm::pi<float>() - a) };
        const float x{ Flipper::r0 * std::cos(angle) };
        const float y{ Flipper::r0 * std::sin(angle) };
        verts[nextVert++] = { { x, y }, color };
    }

    for (int i{ 0 }; i <= numCircleSegments2; ++i)
    {
        const float t{ static_cast<float>(i) / numCircleSegments2 };
        const float angle{ -a + t * 2.0f * a };
        const float x{ d + Flipper::r1 * std::cos(angle) };
        const float y{ Flipper::r1 * std::sin(angle) };
        verts[nextVert++] = { { x, y }, color };
    }

    assert(nextVert == numVerts);

    return verts;
}

FlipperRenderer::FlipperRenderer() : m_vao{ DefaultShader::createVao(makeVerts()) }
{}

void FlipperRenderer::render(const Flipper& flipper, const DefaultShader& s) const
{
    s.setModel(flipper.getTransform());
    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINE_LOOP, 0, numVerts);
}
