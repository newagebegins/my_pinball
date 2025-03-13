#ifndef CIRCLE_RENDERER_H
#define CIRCLE_RENDERER_H

#include "Circle.h"
#include "DefaultShader.h"

class CircleRenderer
{
public:
    CircleRenderer();
    CircleRenderer(const CircleRenderer&) = delete;
    void render(const Circle& c, const DefaultShader& s) const;
private:
    const GLuint m_vao{};
};

#endif
