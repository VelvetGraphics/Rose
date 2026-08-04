#include <Rose/Rose.hpp>

bool SandboxInit(std::vector<std::string>&& args)
{
    Rose::Logger::Trace("Initialized Sandbox!");
    return true;
}

Rose::Main::AppInitFunc Rose::Main::AppInitFn = SandboxInit;

void SandboxShutdown() { Rose::Logger::Trace("Shut down sandbox!"); }

Rose::Main::AppShutdownFunc Rose::Main::AppShutdownFn = SandboxShutdown;
