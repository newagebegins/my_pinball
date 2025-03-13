#ifndef LINE_SEGMENT_RENDERER_H
#define LINE_SEGMENT_RENDERER_H

#include "DefaultShader.h"

#include <glad/glad.h>

#include <vector>

class LineSegmentRenderer
{
public:
    LineSegmentRenderer(const std::vector<glm::vec2>& verts);
    void render(const DefaultShader* s) const;
private:
    const GLuint m_vao{};
    const int m_numVerts{};
};

#endif
