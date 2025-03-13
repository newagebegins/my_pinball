#ifndef FLIPPER_RENDERER_H
#define FLIPPER_RENDERER_H

#include "DefaultShader.h"
#include "Flipper.h"

#include <glad/glad.h>

class FlipperRenderer
{
public:
    FlipperRenderer();
    FlipperRenderer(const FlipperRenderer&) = delete;
    void render(const Flipper& flipper, const DefaultShader* s) const;
private:
    const GLuint m_vao{};
};

#endif
