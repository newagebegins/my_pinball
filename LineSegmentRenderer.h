#ifndef LINE_SEGMENT_RENDERER_H
#define LINE_SEGMENT_RENDERER_H

#include "DefaultShader.h"

#include <glad/glad.h>

#include <vector>

class LineSegmentRenderer
{
public:
    LineSegmentRenderer(const std::vector<DefaultVertex>& verts)
        : m_vao{ DefaultShader::createVao(verts) }
        , m_numVerts{ static_cast<int>(verts.size()) }
    {}

    void render(const DefaultShader& s) const
    {
        s.setModel({ 1.0f });
    
        glBindVertexArray(m_vao);
        glDrawArrays(GL_LINES, 0, m_numVerts);
    }

private:
    const GLuint m_vao{};
    const int m_numVerts{};
};

#endif
