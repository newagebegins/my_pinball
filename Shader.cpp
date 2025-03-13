#include "Shader.h"

#include "File.h"

#include <cassert>
#include <cstdlib>
#include <iostream>

Shader::Shader(GLenum type, const char* path)
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
            default: assert(!"Unreachable"); break;
        }
        std::cerr << " shader error:\n" << infoLog << '\n';
        std::exit(EXIT_FAILURE);
    }
}

Shader::~Shader()
{
    glDeleteShader(id);
}
