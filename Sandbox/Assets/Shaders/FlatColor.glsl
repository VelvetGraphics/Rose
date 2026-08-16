#Type Vertex
#version 460 core

layout (location = 0) in vec3 a_Position;

layout(set = 0, binding = 0) uniform MVP {
    mat4 Model;
    mat4 ViewProjection;
} u_MVP;

void main()
{
    gl_Position = u_MVP.ViewProjection * u_MVP.Model * vec4(a_Position, 1.0);
}

#Type Fragment
#version 460 core

layout (location = 0) out vec4 a_Color;

void main()
{
    a_Color = vec4(0.8, 0.0, 0.8, 1.0);
}