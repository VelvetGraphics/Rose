#include "Renderer.hpp"
#include "Rose/Graphics/GraphicsAPI.hpp"

namespace Rose {
    bool Renderer::Init(GLFWwindow* window)
    {
        s_GraphicsAPI = new GraphicsAPI(window);
        return s_GraphicsAPI->Init();
    }

    void Renderer::Shutdown()
    {
        if (s_GraphicsAPI)
            delete s_GraphicsAPI;
    }

    bool Renderer::BeginFrame() { return s_GraphicsAPI->BeginFrame(); }

    void Renderer::EndFrame() { s_GraphicsAPI->EndFrame(); }
} // namespace Rose
