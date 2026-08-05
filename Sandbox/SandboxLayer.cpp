#include "SandboxLayer.hpp"

void SandboxLayer::OnUpdate(float dt) { Rose::Logger::Info("DeltaTime = {}", dt); }
void SandboxLayer::OnRender() { Rose::Logger::Info("Rendered!"); }
