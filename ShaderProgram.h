#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <glad/glad.h>

class ShaderProgram
{
public:
    const GLuint id{};
    ShaderProgram(const GLchar* vertexCode, const GLchar* fragmentCode);
    ShaderProgram(const ShaderProgram&) = delete;
    ~ShaderProgram();
};

#endif
