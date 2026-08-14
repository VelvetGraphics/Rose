#include "SandboxLayer.hpp"

void SandboxLayer::OnAttach()
{
    m_Mesh = Rose::Mesh::Create("Assets/Models/Monkey.obj");
    m_Shader = Rose::Shader::Create("Assets/Shaders/FlatColor.glsl");
}

void SandboxLayer::OnUpdate(float dt) {}

void SandboxLayer::OnRender()
{
    m_Mesh->Bind();
    m_Shader->Bind();
    m_Scene.Render();
}
