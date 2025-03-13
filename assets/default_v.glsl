#version 410

layout (location = 0) in vec2 pos;

uniform mat3 model;
uniform mat3 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(view * model * vec3(pos, 1.0), 1.0);
}
