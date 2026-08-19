#pragma once
#include "Rose/Graphics/ViewportImage.hpp"

struct GLFWwindow;

namespace Rose {
    class GraphicsAPI;
    class VulkanAPI;
    class Renderer
    {
    public:
        static bool Init(GLFWwindow* window);
        static void Shutdown();

        static bool BeginFrame();
        static void EndFrame();

        static void WaitDeviceIdle();
        static void SetViewport(ViewportImage& viewport);

    private:
        inline static GraphicsAPI* s_GraphicsAPI = nullptr;
    };
} // namespace Rose
