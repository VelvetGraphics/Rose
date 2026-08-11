#pragma once
#include "Rose/Graphics/VulkanInclude.hpp"

namespace Rose {
    class GraphicsAPI
    {
    public:
        explicit GraphicsAPI(GLFWwindow* window);
        ~GraphicsAPI();

        bool Init();

        bool BeginFrame();
        void EndFrame();

    private:
        void BootStrap();
        void CreateCmdBuffers();

    private:
        GLFWwindow* m_Window = nullptr;
        vk::SurfaceKHR m_Surface;

        vkb::Instance m_Instance;

        vkb::Device m_VkbDevice;
        vk::Device m_Device;

        vk::Queue m_GraphicsQueue;

        vkb::Swapchain m_SwapChain;

        vk::CommandPool m_CmdPool;
        vk::CommandBuffer m_CmdBuffer;
    };
} // namespace Rose
