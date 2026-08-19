#include "ViewportLayer.hpp"

#include "Thorn/ThornApp.hpp"

namespace Rose {
    ViewportLayer::ViewportLayer()
    {
        auto& window = Main::GetWindow();
        m_Camera.SetPerspective(45.0f, static_cast<float>(window->GetWidth()) / static_cast<float>(window->GetHeight()),
                                0.01f, 1000.0f);

        Main::GetEventBus().Observe<MouseMovedEvent>([this](MouseMovedEvent& e) { OnMouseMoved(e); });
    }

    void ViewportLayer::OnUpdate(float dt)
    {
        constexpr float camSpeed = 5.0f;

        glm::vec3 velocity{0.0f};

        if (Input::IsKeyPressed(Key::W))
            velocity += camSpeed * m_Camera.GetForward();
        if (Input::IsKeyPressed(Key::S))
            velocity -= camSpeed * m_Camera.GetForward();
        if (Input::IsKeyPressed(Key::A))
            velocity -= camSpeed * m_Camera.GetRight();
        if (Input::IsKeyPressed(Key::D))
            velocity += camSpeed * m_Camera.GetRight();

        constexpr glm::vec3 worldUp{0.0f, 1.0f, 0.0f};

        if (Input::IsKeyPressed(Key::Q))
            velocity -= camSpeed * worldUp;
        if (Input::IsKeyPressed(Key::E))
            velocity += camSpeed * worldUp;

        if (glm::length(velocity))
            velocity = glm::normalize(velocity);
        else
            velocity = glm::vec3{0.0f};

        velocity *= camSpeed * dt;

        if (m_Selected)
        {
            m_Camera.Position += velocity;

            if (Input::IsKeyPressed(Key::ESCAPE))
                Main::GetWindow()->SetMouseMode(MouseMode::Normal);
            else if (Input::IsKeyPressed(Key::TAB))
                Main::GetWindow()->SetMouseMode(MouseMode::Captured);
        }
    }

    void ViewportLayer::OnRender() { m_Scene.Render(); }

    void ViewportLayer::OnImGuiRender()
    {
        ImGui::DockSpaceOverViewport();
        Ref<ViewportImage>& viewport = ThornApp::GetViewport();

        bool open = true;
        ImGui::Begin("Viewport", &open, ImGuiWindowFlags_NoScrollbar);
        m_Selected = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        ImGui::Image(viewport->GetTextureID(), viewport->Size());
        ImGui::End();
    }

    void ViewportLayer::OnMouseMoved(MouseMovedEvent& e)
    {
        if (m_Selected && Input::IsMouseButtonPressed(Mouse::MIDDLE) &&
            Main::GetWindow()->GetMouseMode() == MouseMode::Captured)
        {
            glm::vec2 posChange = Input::GetMousePos() - m_MousePos;
            constexpr float mouseSensitivity = 0.2f;

            m_Camera.Yaw -= posChange.x * mouseSensitivity;
            m_Camera.Pitch -= posChange.y * mouseSensitivity;
        }

        m_MousePos = Input::GetMousePos();
    }
} // namespace Rose
