#Type Vertex
#version 460 core

layout (location = 0) in vec3 a_Pos;

layout(binding = 0) uniform UniformBufferObject {
    mat4 Model;
    mat4 ViewProjection;
} u_MVP;

void main()
{
    gl_Position = u_MVP.ViewProjection * u_MVP.Model * vec4(a_Pos, 1.0);
}

#Type Fragment
#version 460 core

layout (location = 0) out vec4 o_Color;

void main()
{
    o_Color = vec4(0.5, 0.2, 0.4, 1.0);
}