#ifndef CIRCLE_RENDERER_H
#define CIRCLE_RENDERER_H

#include "Circle.h"
#include "DefaultShader.h"

#include <glm/ext/scalar_constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>

class CircleRenderer
{
public:
    CircleRenderer() : m_vao{ DefaultShader::createVao(makeVerts()) }
    {}

    CircleRenderer(const CircleRenderer&) = delete;

    void render(const Circle& c, const DefaultShader& s) const
    {
        glm::mat3 model{ 1.0f };
        model = glm::translate(model, c.center);
        model = glm::scale(model, glm::vec2{ c.radius });
        s.setModel(model);

        glBindVertexArray(m_vao);
        glDrawArrays(GL_LINE_LOOP, 0, numVerts);
    }
private:
    static constexpr int numVerts{ 64 };

    static std::vector<DefaultVertex> makeVerts()
    {
        std::vector<DefaultVertex> verts(numVerts);

        for (std::size_t i{ 0 }; i < numVerts; ++i)
        {
            const float t{ static_cast<float>(i) / numVerts };
            const float angle{ t * 2.0f * glm::pi<float>() };
            verts[i] = {
                { std::cos(angle), std::sin(angle) },
                { 1.0f, 1.0f, 1.0f },
            };
        }

        return verts;
    }

    const GLuint m_vao{};
};

#endif
