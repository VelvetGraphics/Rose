#pragma once
#include <Rose/Rose.hpp>

class SandboxLayer : public Rose::Layer
{
public:
    void OnAttach() override;
    void OnUpdate(float dt) override;
    void OnRender() override;

    void OnMouseMoved(Rose::MouseMovedEvent& e);

private:
    glm::vec2 m_MousePos;

    Rose::Scene m_Scene;
    Rose::Camera m_Camera;
};
