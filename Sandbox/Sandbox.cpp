#include <Rose/Rose.hpp>

#include "SandboxLayer.hpp"

bool SandboxInit(std::vector<std::string>&& args)
{
    Rose::Main::Instance()->PushLayer(new SandboxLayer);
    return true;
}

Rose::Main::AppInitFunc Rose::Main::AppInitFn = SandboxInit;

void SandboxShutdown() {}

Rose::Main::AppShutdownFunc Rose::Main::AppShutdownFn = SandboxShutdown;

Rose::WindowInfo SandboxWindowInfoFn() { return {}; }

Rose::Window::WindowInfoFunc Rose::Window::WindowInfoFn = SandboxWindowInfoFn;
