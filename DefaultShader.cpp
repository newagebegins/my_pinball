#include "DefaultShader.h"

GLuint DefaultShader::createVao(const std::vector<glm::vec2>& verts)
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

DefaultShader::DefaultShader()
    : m_program{ "assets/default_v.glsl", "assets/default_f.glsl" }
    , m_modelLoc{ glGetUniformLocation(m_program.id, "model") }
    , m_viewLoc{ glGetUniformLocation(m_program.id, "view") }
    , m_projectionLoc{ glGetUniformLocation(m_program.id, "projection") }
{
    assert(m_modelLoc >= 0);
    assert(m_viewLoc >= 0);
    assert(m_projectionLoc >= 0);
}

void DefaultShader::use() const
{
    glUseProgram(m_program.id);
}

void DefaultShader::setModel(const glm::mat3& model) const
{
    glUniformMatrix3fv(m_modelLoc, 1, GL_FALSE, &model[0][0]);
}

void DefaultShader::setView(const glm::mat3& view) const
{
    glUniformMatrix3fv(m_viewLoc, 1, GL_FALSE, &view[0][0]);
}

void DefaultShader::setProjection(const glm::mat4& projection) const
{
    glUniformMatrix4fv(m_projectionLoc, 1, GL_FALSE, &projection[0][0]);
}
