#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

class Shader
{
public:
    const GLuint id{};
    Shader(GLenum type, const char* path);
    Shader(const Shader&) = delete;
    ~Shader();
};

#endif
