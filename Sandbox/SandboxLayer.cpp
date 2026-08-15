#include "SandboxLayer.hpp"

namespace {
    struct MVP
    {
        glm::mat4 Model;
        glm::mat4 ViewProjection;
    };
} // namespace

void SandboxLayer::OnAttach()
{
    Ref<Rose::Window> window = Rose::Main::GetWindow();
    m_Camera.SetPerspective(45.0f, static_cast<float>(window->GetWidth()) / static_cast<float>(window->GetHeight()),
                            0.01f, 1000.0f);

    m_Monkey = m_Scene.CreateEntity("Monkey");

    m_MonkeyTransform = &m_Scene.AddComponent<Rose::TransformComponent>(m_Monkey);
    m_MonkeyTransform->position = glm::vec3{0.0f, 0.0f, -5.0f};

    auto& renderer3DComponent = m_Scene.AddComponent<Rose::Renderer3DComponent>(m_Monkey);

    renderer3DComponent.Mesh = Rose::Mesh::Create("Assets/Models/Monkey.obj");
    renderer3DComponent.Shader = Rose::Shader::Create("Assets/Shaders/FlatColor.glsl");

    m_MonkeyShader = renderer3DComponent.Shader;
}

void SandboxLayer::OnUpdate(float dt)
{
    if (Rose::Input::IsKeyPressed(Rose::Key::I))
        m_MonkeyTransform->position.z -= 1 * dt;
    if (Rose::Input::IsKeyPressed(Rose::Key::K))
        m_MonkeyTransform->position.z += 1 * dt;
    if (Rose::Input::IsKeyPressed(Rose::Key::J))
        m_MonkeyTransform->position.x -= 1 * dt;
    if (Rose::Input::IsKeyPressed(Rose::Key::L))
        m_MonkeyTransform->position.x += 1 * dt;
}

void SandboxLayer::OnRender()
{
    MVP mvp;
    mvp.Model = m_MonkeyTransform->Transform();
    mvp.ViewProjection = m_Camera.GetProjectionMatrix() * m_Camera.GetViewMatrix();
    m_MonkeyShader->SetUBO<MVP>({0, 0}, &mvp);
    m_Scene.Render();
}
