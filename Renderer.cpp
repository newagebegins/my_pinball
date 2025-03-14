#include "Renderer.h"

#include "Constants.h"

#include <glm/ext/matrix_clip_space.hpp> // for glm::ortho()

Renderer::Renderer(const std::vector<DefaultVertex>& verts)
    : m_lineSegmentRenderer{ verts }
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

    m_circleRenderer.render(game.circle, m_defShader);

    for (const auto& flipper : game.flippers)
    {
        m_flipperRenderer.render(flipper, m_defShader);
    }

    m_lineSegmentRenderer.render(m_defShader);
}
