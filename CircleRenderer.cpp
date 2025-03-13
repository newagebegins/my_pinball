#include "CircleRenderer.h"

#include <glm/ext/scalar_constants.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>

constexpr int numVerts{ 32 };

static std::vector<glm::vec2> makeVerts()
{
    std::vector<glm::vec2> verts(numVerts);

    for (std::size_t i{ 0 }; i < numVerts; ++i)
    {
        const float t{ static_cast<float>(i) / numVerts };
        const float angle{ t * 2.0f * glm::pi<float>() };
        verts[i] = { std::cos(angle), std::sin(angle) };
    }

    return verts;
}

CircleRenderer::CircleRenderer() : m_vao{ DefaultShader::createVao(makeVerts()) }
{}

void CircleRenderer::render(const Circle& c, const DefaultShader& s) const
{
    glm::mat3 model{ 1.0f };
    model = glm::translate(model, c.center);
    model = glm::scale(model, glm::vec2{ c.radius });
    s.setModel(model);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINE_LOOP, 0, numVerts);
}
