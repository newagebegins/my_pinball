#include "Shader.h"

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

namespace File
{
    std::string readEntireFile(const char* path)
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
            default:                 std::cerr << "???"; break;
        }
        std::cerr << " shader error:\n" << infoLog << '\n';
        std::exit(EXIT_FAILURE);
    }
}

Shader::~Shader()
{
    glDeleteShader(id);
}
