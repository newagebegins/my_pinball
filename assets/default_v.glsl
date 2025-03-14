#version 410

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec3 inCol;

out vec3 col;

uniform mat3 model;
uniform mat3 view;
uniform mat4 projection;

void main()
{
    col = inCol;
    gl_Position = projection * vec4(view * model * vec3(inPos, 1.0), 1.0);
}
