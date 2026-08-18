#pragma once

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

    private:
        inline static GraphicsAPI* s_GraphicsAPI = nullptr;
    };
} // namespace Rose
