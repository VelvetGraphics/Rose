#include "Renderer.hpp"

#include "Platform/Vulkan/Graphics/VulkanAPI.hpp"

namespace Rose {
    bool Renderer::Init(GLFWwindow* window)
    {
        s_GraphicsAPI = GraphicsAPI::Create(window);
        s_GraphicsAPI->MakeContextCurrent();
        return s_GraphicsAPI->Init();
    }

    void Renderer::Shutdown() { delete s_GraphicsAPI; }

    bool Renderer::BeginFrame() { return s_GraphicsAPI->BeginFrame(); }

    void Renderer::EndFrame() { s_GraphicsAPI->EndFrame(); }

    void Renderer::WaitDeviceIdle() { s_GraphicsAPI->WaitDeviceIdle(); }
} // namespace Rose
