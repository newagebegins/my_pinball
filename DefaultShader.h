#ifndef DEFAULT_SHADER_H
#define DEFAULT_SHADER_H

#include "DefaultVertex.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

namespace File
{
    inline std::string readEntireFile(const char* path)
    {
        std::ifstream ifs{ path };
        if (!ifs.is_open())
        {
            std::cerr << "Failed to open the file: " << path << '\n';
            std::exit(EXIT_FAILURE);
        }
        std::stringstream s{};
        s << ifs.rdbuf();
        return s.str();
    }
}

class Shader
{
public:
    const GLuint id{};
    Shader(GLenum type, const char* path)
        : id{ glCreateShader(type) }
    {
        std::string codeStr{ File::readEntireFile(path) };
        const char* code{ codeStr.c_str() };

        glShaderSource(id, 1, &code, nullptr);
        glCompileShader(id);

        GLint success{};
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            GLchar infoLog[512];
            glGetShaderInfoLog(id, sizeof(infoLog), nullptr, infoLog);
            switch (type)
            {
                case GL_VERTEX_SHADER:   std::cerr << "Vertex"; break;
                case GL_FRAGMENT_SHADER: std::cerr << "Fragment"; break;
                default:                 std::cerr << "???"; break;
            }
            std::cerr << " shader error:\n" << infoLog << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    Shader(const Shader&) = delete;

    ~Shader()
    {
        glDeleteShader(id);
    }
};

inline GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath)
{
    const Shader vs{ GL_VERTEX_SHADER, vertexPath };
    const Shader fs{ GL_FRAGMENT_SHADER, fragmentPath };

    const GLuint id{ glCreateProgram() };
    glAttachShader(id, vs.id);
    glAttachShader(id, fs.id);
    glLinkProgram(id);

    GLint success{};
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetProgramInfoLog(id, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Program link error:\n" << infoLog << '\n';
        std::exit(EXIT_FAILURE);
    }

    return id;
}

class ShaderProgram
{
public:
    const GLuint id{};
    ShaderProgram(const char* vertexPath, const char* fragmentPath)
        : id{ createShaderProgram(vertexPath, fragmentPath) }
    {}

    ShaderProgram(const ShaderProgram&) = delete;

    ~ShaderProgram()
    {
        glDeleteProgram(id);
    }
};

class DefaultShader
{
public:
    static GLuint createVao(const std::vector<DefaultVertex>& verts);

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
