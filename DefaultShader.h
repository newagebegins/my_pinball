#ifndef DEFAULT_SHADER_H
#define DEFAULT_SHADER_H

#include "ShaderProgram.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>

class DefaultShader
{
public:
    static GLuint createVao(const std::vector<glm::vec2>& verts);

    DefaultShader();
    DefaultShader(const DefaultShader&) = delete;

    void use() const;
    void setModel(const glm::mat3& model) const;
    void setView(const glm::mat3& view) const;
    void setProjection(const glm::mat4& projection) const;

private:
    const ShaderProgram m_program;
    const GLint m_modelLoc{};
    const GLint m_viewLoc{};
    const GLint m_projectionLoc{};
};

#endif
