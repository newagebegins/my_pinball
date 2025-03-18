#include "ShaderProgram.h"

#include <glad/glad.h>

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
