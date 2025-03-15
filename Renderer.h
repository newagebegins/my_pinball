#ifndef RENDERER_H
#define RENDERER_H

#include "CircleRenderer.h"
#include "DefaultShader.h"
#include "FlipperRenderer.h"
#include "Game.h"
#include "LineSegmentRenderer.h"

class Renderer
{
public:
    Renderer(const Game& game);
    Renderer(const Renderer&) = delete;
    void render(const Game& game) const;
private:
    DefaultShader m_defShader{};
    CircleRenderer m_circleRenderer{};
    FlipperRenderer m_flipperRenderer{};
    LineSegmentRenderer m_lineSegmentRenderer;
};

#endif
