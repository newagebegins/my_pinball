#include "renderer.h"

#include <glm/glm.hpp>                   // for glm types
#include <glm/ext/matrix_clip_space.hpp> // for glm::ortho()

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp> // for glm::translate(), glm::rotate(), glm::scale()

#include <glad/glad.h>

#include <cassert>
#include <cmath> // for std::sin(), std::cos(), std::acos()
#include <cstdlib> // for std::exit(), EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>
#include <vector>

GLuint createShaderProgram(const GLchar* vertexCode, const GLchar* fragmentCode)
{
    GLint success{};
    GLchar infoLog[512];

    const GLuint vs{ glCreateShader(GL_VERTEX_SHADER) };
    glShaderSource(vs, 1, &vertexCode, nullptr);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vs, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Vertex shader error:\n" << infoLog << '\n';
        std::exit(EXIT_FAILURE);
    }

    const GLuint fs{ glCreateShader(GL_FRAGMENT_SHADER) };
    glShaderSource(fs, 1, &fragmentCode, nullptr);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fs, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Fragment shader error:\n" << infoLog << '\n';
        std::exit(EXIT_FAILURE);
    }

    const GLuint program{ glCreateProgram() };
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Program link error:\n" << infoLog << '\n';
        std::exit(EXIT_FAILURE);
    }

    glDeleteShader(fs);
    glDeleteShader(vs);

    return program;
}

class DefaultShader
{
public:
    DefaultShader()
        : m_program{ createShaderProgram(vertexCode, fragmentCode) }
        , m_modelLoc{ glGetUniformLocation(m_program, "model") }
        , m_viewLoc{ glGetUniformLocation(m_program, "view") }
        , m_projectionLoc{ glGetUniformLocation(m_program, "projection") }
    {
        assert(m_modelLoc >= 0);
        assert(m_viewLoc >= 0);
        assert(m_projectionLoc >= 0);
    }

    void use() const
    {
        glUseProgram(m_program);
    }

    void setModel(const glm::mat3& model) const
    {
        glUniformMatrix3fv(m_modelLoc, 1, GL_FALSE, &model[0][0]);
    }

    void setView(const glm::mat3& view) const
    {
        glUniformMatrix3fv(m_viewLoc, 1, GL_FALSE, &view[0][0]);
    }

    void setProjection(const glm::mat4& projection) const
    {
        glUniformMatrix4fv(m_projectionLoc, 1, GL_FALSE, &projection[0][0]);
    }

    static GLuint createVao(const std::vector<glm::vec2>& verts)
    {
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        const GLsizeiptr size{ static_cast<GLsizeiptr>(verts.size() * sizeof(verts[0])) };
        glBufferData(GL_ARRAY_BUFFER, size, verts.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(verts[0]), nullptr);
        glEnableVertexAttribArray(0);
        return vao;
    }

private:
    const GLuint m_program;
    const GLint m_modelLoc;
    const GLint m_viewLoc;
    const GLint m_projectionLoc;

    inline static const GLchar* const vertexCode{ R"(
#version 410

layout (location = 0) in vec2 pos;

uniform mat3 model;
uniform mat3 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * vec4(view * model * vec3(pos, 1.0), 1.0);
}
)" };

    inline static const GLchar* const fragmentCode{ R"(
#version 410

out vec4 fragColor;

void main()
{
	fragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)" };
};

class CircleRenderer
{
public:
    CircleRenderer()
    {
        std::vector<glm::vec2> verts(numVerts);

        for (std::size_t i{ 0 }; i < numVerts; ++i)
        {
            const float t{ static_cast<float>(i) / numVerts };
            const float angle{ t * 2.0f * glm::pi<float>() };
            verts[i] = { std::cos(angle), std::sin(angle) };
        }

        m_vao = DefaultShader::createVao(verts);
    }

    void render(const Circle& c, const DefaultShader& s)
    {
        glm::mat3 model{ glm::mat3(1.0f) };
        model = glm::translate(model, c.center);
        model = glm::scale(model, glm::vec2(c.radius));
        glBindVertexArray(m_vao);
        s.setModel(model);
        glDrawArrays(GL_LINE_LOOP, 0, numVerts);
    }

private:
    static constexpr int numVerts{ 32 };
    GLuint m_vao;
};

class FlipperRenderer
{
public:
    FlipperRenderer()
    {
        constexpr float r0{ 1.1f };
        constexpr float r1{ 0.7f };
        constexpr float width{ 8.0f };
        constexpr float d{ width - r0 - r1 };
        constexpr float cosA{ (r0 - r1) / d };
        const float a{ std::acos(cosA) };

        std::vector<glm::vec2> vertices(numVerts);

        std::size_t nextVertex{ 0 };

        for (int i{ 0 }; i <= numCircleSegments1; ++i)
        {
            const float t{ static_cast<float>(i) / numCircleSegments1 };
            const float angle{ a + 2.0f * t * (glm::pi<float>() - a) };
            const float x{ r0 * std::cos(angle) };
            const float y{ r0 * std::sin(angle) };
            vertices[nextVertex++] = { x, y };
        }

        for (int i{ 0 }; i <= numCircleSegments2; ++i)
        {
            const float t{ static_cast<float>(i) / numCircleSegments2 };
            const float angle{ -a + t * 2.0f * a };
            const float x{ d + r1 * std::cos(angle) };
            const float y{ r1 * std::sin(angle) };
            vertices[nextVertex++] = { x, y };
        }

        assert(nextVertex == numVerts);

        m_vao = DefaultShader::createVao(vertices);
    }

    void render(const Flipper& flipper, const DefaultShader& s)
    {
        glBindVertexArray(m_vao);
        s.setModel(flipper.transform);
        glDrawArrays(GL_LINE_LOOP, 0, numVerts);
    }
private:
    static constexpr int numCircleSegments1{ 16 };
    static constexpr int numCircleSegments2{ 8 };
    static constexpr int numVerts{ (numCircleSegments1 + 1) + (numCircleSegments2 + 1) };

    GLuint m_vao;
};

class Renderer
{
public:
    Renderer()
    {
        const glm::mat3 view{ glm::translate(glm::mat3{ 1.0f }, { 0.0f, -20.0f }) };

        constexpr float zoom{ 32.0f };
        const glm::mat4 projection{ glm::ortho(-1.0f * zoom, 1.0f * zoom, -1.0f * zoom, 1.0f * zoom, -1.0f, 1.0f) };

        m_defShader.use();
        m_defShader.setView(view);
        m_defShader.setProjection(projection);
    }

    void render(const Scene& scene)
    {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_circleRenderer.render(scene.circle, m_defShader);

        for (const auto& flipper : scene.flippers)
        {
            m_flipperRenderer.render(flipper, m_defShader);
        }
    }

private:
    DefaultShader m_defShader;
    CircleRenderer m_circleRenderer;
    FlipperRenderer m_flipperRenderer;
};

void render(const Scene& scene)
{
    static Renderer renderer;
    renderer.render(scene);
}
