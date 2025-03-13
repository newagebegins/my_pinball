#include "ShaderProgram.h"

#include "Shader.h"

#include <cstdlib>
#include <iostream>

GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath)
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

ShaderProgram::ShaderProgram(const GLchar* vertexCode, const GLchar* fragmentCode)
    : id{ createShaderProgram(vertexCode, fragmentCode) }
{}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(id);
}
