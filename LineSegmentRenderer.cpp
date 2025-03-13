#include "LineSegmentRenderer.h"

LineSegmentRenderer::LineSegmentRenderer(const std::vector<glm::vec2>& verts)
    : m_vao{ DefaultShader::createVao(verts) }
    , m_numVerts{ static_cast<int>(verts.size()) }
{}

void LineSegmentRenderer::render(const DefaultShader* s) const
{
    s->setModel({ 1.0f });
    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINES, 0, m_numVerts);
}
