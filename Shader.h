#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

class Shader
{
public:
    const GLuint id{};
    Shader(GLenum type, const GLchar* code);
    Shader(const Shader&) = delete;
    ~Shader();
};

#endif
