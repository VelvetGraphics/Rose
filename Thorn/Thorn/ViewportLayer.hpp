#pragma once
#include <Rose/Rose.hpp>

namespace Rose {
    class ViewportLayer : public Layer
    {
    public:
        ViewportLayer();

        void OnUpdate(float dt) override;
        void OnRender() override;
        void OnImGuiRender() override;

        void OnMouseMoved(MouseMovedEvent& e);

        Scene& GetScene() { return m_Scene; }

    private:
        Scene m_Scene;
        Camera m_Camera;

        glm::vec2 m_MousePos;
        bool m_Selected = false;
    };
} // namespace Rose
