#Type Vertex
#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

void main()
{
    gl_Position = vec4(a_Position, 1.0);
}

#Type Fragment
#version 460 core

layout (location = 0) out vec4 a_Color;

void main()
{
    a_Color = vec4(0.8, 0.0, 0.8, 1.0);
}