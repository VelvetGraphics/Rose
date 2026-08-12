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
        void CreateSyncObjects();

        void RecreateSwapChain();

    private:
        GLFWwindow* m_Window = nullptr;
        vk::SurfaceKHR m_Surface;

        vkb::Instance m_Instance;

        vkb::Device m_VkbDevice;
        vk::Device m_Device;

        vk::Queue m_GraphicsQueue;

        vkb::Swapchain m_SwapChain;
        std::vector<vk::Image> m_SwapChainImages;
        std::vector<vk::ImageView> m_SwapChainImageViews;
        U32 m_ImageIndex = 0;

        vk::CommandPool m_CmdPool;
        std::vector<vk::CommandBuffer> m_CmdBuffers;

        std::vector<vk::Semaphore> m_ImageAvailableSemaphores;
        std::vector<vk::Semaphore> m_RendererFinishedSemaphores;
        std::vector<vk::Fence> m_InFlightFences;

        U32 m_FrameIndex = 0;
    };
} // namespace Rose
