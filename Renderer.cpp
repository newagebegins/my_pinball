#include "Renderer.h"

#include "Constants.h"

#include <glm/ext/matrix_clip_space.hpp> // for glm::ortho()

std::vector<DefaultVertex> makeVerts(const Game& game)
{
    std::vector<DefaultVertex> verts{ game.lines };

    glm::vec3 color{1.0f, 1.0f, 1.0f};

    for (const auto& arc : game.arcs)
    {
        float start{ arc.start };
        float end{ arc.end };
        if (arc.end < arc.start)
        {
            end += twoPi;
        }

        int numSteps{ 32 };
        verts.push_back({{
            arc.p.x + std::cos(start) * arc.r,
            arc.p.y + std::sin(start) * arc.r,
        }, color});

        for (int i{ 1 }; i < numSteps; ++i)
        {
            const float t{ static_cast<float>(i) / numSteps };
            const float angle{ glm::mix(start, end, t) };
            float x = arc.p.x + std::cos(angle) * arc.r;
            float y = arc.p.y + std::sin(angle) * arc.r;
            verts.push_back({{x,y}, color});
            verts.push_back({{x,y}, color});
        }

        verts.push_back({{
            arc.p.x + std::cos(end) * arc.r,
            arc.p.y + std::sin(end) * arc.r,
        }, color});
    }

    return verts;
}

Renderer::Renderer(const Game& game)
    : m_lineSegmentRenderer{ makeVerts(game) }
{
    const glm::mat3 identity{ 1.0f };
    const glm::mat4 projection{ glm::ortho(Constants::worldL, Constants::worldR, Constants::worldB, Constants::worldT, -1.0f, 1.0f) };

    m_defShader.use();
    m_defShader.setView(identity);
    m_defShader.setProjection(projection);
}

void Renderer::render(const Game& game) const
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (const auto& c : game.circles)
    {
        m_circleRenderer.render(c, m_defShader);
    }

    for (const auto& flipper : game.flippers)
    {
        m_flipperRenderer.render(flipper, m_defShader);
    }

    m_lineSegmentRenderer.render(m_defShader);
}
