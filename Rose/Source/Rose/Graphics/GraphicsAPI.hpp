#pragma once

#include <GLFW/glfw3.h>

namespace Rose {
    class GraphicsAPI
    {
    public:
        enum API : U8
        {
            Vulkan
        };

    public:
        virtual ~GraphicsAPI() = default;

        static GraphicsAPI* Create(GLFWwindow* window);

        virtual bool Init() = 0;

        virtual bool BeginFrame() = 0;
        virtual void EndFrame() = 0;

        virtual void MakeContextCurrent() = 0;
        virtual void WaitDeviceIdle() const = 0;

        static API GetAPI() { return s_API; }

    protected:
        GraphicsAPI() = default;

    private:
        inline static API s_API = Vulkan;
    };
} // namespace Rose
