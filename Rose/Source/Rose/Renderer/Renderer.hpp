#pragma once

struct GLFWwindow;

namespace Rose {
    class GraphicsAPI;
    class Renderer
    {
    public:
        static bool Init(GLFWwindow* window);
        static void Shutdown();

        static bool BeginFrame();
        static void EndFrame();

    private:
        inline static GraphicsAPI* s_GraphicsAPI = nullptr;
    };
} // namespace Rose
