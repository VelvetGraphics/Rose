#include "SandboxLayer.hpp"

void SandboxLayer::OnAttach() { m_Shader = Rose::Shader::Create("Assets/Shaders/FlatColor.glsl"); }

void SandboxLayer::OnUpdate(float dt) {}

void SandboxLayer::OnRender() { m_Scene.Render(); }
