#pragma once
#include "Rose/ImGui/ImGuiAPI.hpp"
#include "Rose/Renderer/Renderer.hpp"

namespace Rose {
    class VulkanImGuiAPI
    {
    public:
        static void Init(GLFWwindow* window);
        static void Shutdown();

        static void BeginFrame();
        static void EndFrame();
    };
} // namespace Rose
