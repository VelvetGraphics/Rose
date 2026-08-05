#pragma once
#include <Rose/Rose.hpp>

class SandboxLayer : public Rose::Layer
{
public:
    void OnUpdate(float dt) override;
    void OnRender() override;

private:
    Rose::Scene m_Scene;
};
