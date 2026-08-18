#include "SandboxLayer.hpp"

namespace {
    struct MVP
    {
        glm::mat4 Model;
        glm::mat4 ViewProjection;
    };
} // namespace

SandboxLayer::SandboxLayer()
{
    Rose::Main::GetEventBus().Observe<Rose::MouseMovedEvent>([this](Rose::MouseMovedEvent& e) { OnMouseMoved(e); });

    m_MousePos = Rose::Input::GetMousePos();

    Ref<Rose::Window> window = Rose::Main::GetWindow();
    m_Camera.SetPerspective(45.0f, static_cast<float>(window->GetWidth()) / static_cast<float>(window->GetHeight()),
                            0.01f, 1000.0f);

    Ref<Rose::Mesh> mesh = Rose::Mesh::Create("Assets/Models/Monkey.obj");
    m_Texture = Rose::Texture::Create("Assets/Textures/Disco.jpg", {});

    for (U32 z = 0; z < 5; z++)
    {
        for (U32 y = 0; y < 5; y++)
        {
            for (U32 x = 0; x < 5; x++)
            {
                entt::entity monkey = m_Scene.CreateEntity("Monkey");

                auto& monkeyTransform = m_Scene.AddComponent<Rose::TransformComponent>(monkey);
                monkeyTransform.position = glm::vec3{static_cast<float>(x) * 3.0f, static_cast<float>(y) * 3.0f,
                                                     static_cast<float>(z) * 3.0f};

                auto& renderer3DComponent = m_Scene.AddComponent<Rose::Renderer3DComponent>(monkey);

                renderer3DComponent.Mesh = mesh;
                renderer3DComponent.Shader = Rose::Shader::Create("Assets/Shaders/FlatColor.glsl");
            }
        }
    }
}

void SandboxLayer::OnUpdate(float dt)
{
    constexpr float camSpeed = 5.0f;

    glm::vec3 velocity{0.0f};

    if (Rose::Input::IsKeyPressed(Rose::Key::W))
        velocity += camSpeed * m_Camera.GetForward();
    if (Rose::Input::IsKeyPressed(Rose::Key::S))
        velocity -= camSpeed * m_Camera.GetForward();
    if (Rose::Input::IsKeyPressed(Rose::Key::A))
        velocity -= camSpeed * m_Camera.GetRight();
    if (Rose::Input::IsKeyPressed(Rose::Key::D))
        velocity += camSpeed * m_Camera.GetRight();

    constexpr glm::vec3 worldUp{0.0f, 1.0f, 0.0f};

    if (Rose::Input::IsKeyPressed(Rose::Key::Q))
        velocity -= camSpeed * worldUp;
    if (Rose::Input::IsKeyPressed(Rose::Key::E))
        velocity += camSpeed * worldUp;

    if (glm::length(velocity))
        velocity = glm::normalize(velocity);
    else
        velocity = glm::vec3{0.0f};

    velocity *= camSpeed * dt;

    m_Camera.Position += velocity;

    if (Rose::Input::IsKeyPressed(Rose::Key::ESCAPE))
        Rose::Main::GetWindow()->SetMouseMode(Rose::MouseMode::Normal);
    else if (Rose::Input::IsKeyPressed(Rose::Key::TAB))
        Rose::Main::GetWindow()->SetMouseMode(Rose::MouseMode::Captured);
}

void SandboxLayer::OnRender()
{
    auto view = m_Scene.GetRegistry().view<Rose::Renderer3DComponent>();
    for (auto entity : view)
    {
        MVP mvp{};
        mvp.Model = m_Scene.GetComponent<Rose::TransformComponent>(entity).Transform();
        mvp.ViewProjection = m_Camera.GetProjectionMatrix() * m_Camera.GetViewMatrix();

        auto& renderer3DComponent = m_Scene.GetComponent<Rose::Renderer3DComponent>(entity);
        renderer3DComponent.Shader->SetUBO({0, 0}, &mvp, sizeof(MVP));
        renderer3DComponent.Shader->SetTexture({0, 1}, m_Texture);
    }

    m_Scene.Render();
}

void SandboxLayer::OnMouseMoved(Rose::MouseMovedEvent& e)
{
    if (Rose::Input::IsMouseButtonPressed(Rose::Mouse::MIDDLE) &&
        Rose::Main::GetWindow()->GetMouseMode() == Rose::MouseMode::Captured)
    {
        glm::vec2 posChange = Rose::Input::GetMousePos() - m_MousePos;
        constexpr float mouseSensitivity = 0.2f;

        m_Camera.Yaw -= posChange.x * mouseSensitivity;
        m_Camera.Pitch -= posChange.y * mouseSensitivity;
    }

    m_MousePos = Rose::Input::GetMousePos();
}
