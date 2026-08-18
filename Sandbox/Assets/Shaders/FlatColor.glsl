#Type Vertex
#version 460 core

layout (location = 0) in vec3 a_Position;

layout (location = 2) in vec2 a_TexCoord;
layout (location = 2) out vec2 v_TexCoord;

layout(set = 0, binding = 0) uniform MVP {
    mat4 Model;
    mat4 ViewProjection;
} u_MVP;

void main()
{
    gl_Position = u_MVP.ViewProjection * u_MVP.Model * vec4(a_Position, 1.0);

    v_TexCoord = a_TexCoord;
}

#Type Fragment
#version 460 core

layout (location = 0) out vec4 a_Color;

layout (location = 2) in vec2 v_TexCoord;
layout (set = 0, binding = 1) uniform sampler2D u_Texture;

void main()
{
    a_Color = texture(u_Texture, v_TexCoord);
}