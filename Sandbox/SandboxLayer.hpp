#pragma once
#include <Rose/Rose.hpp>

class SandboxLayer : public Rose::Layer
{
public:
    void OnAttach() override;
    void OnUpdate(float dt) override;
    void OnRender() override;

private:
    Rose::Scene m_Scene;
    Rose::Camera m_Camera;

    entt::entity m_Monkey = entt::null;
    Rose::TransformComponent* m_MonkeyTransform = nullptr;
    Ref<Rose::Shader> m_MonkeyShader;
};
