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

        void MakeContextCurrent() { s_CurrentContext = this; }

        static vk::Device Device() { return s_CurrentContext->m_Device; }

        static vk::Format SwapChainSurfaceFormat();
        static vk::Extent2D SwapChainExtent() { return s_CurrentContext->m_SwapChain.extent; }

        static void Submit(std::function<void(vk::CommandBuffer)> cmd);
        void ExecCommands();

    private:
        void BootStrap();
        void CreateCmdBuffers();
        void CreateSyncObjects();

        void RecreateSwapChain();

    private:
        inline static GraphicsAPI* s_CurrentContext = nullptr;

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
        std::vector<std::function<void(vk::CommandBuffer)>> m_Commands;

        std::vector<vk::Semaphore> m_ImageAvailableSemaphores;
        std::vector<vk::Semaphore> m_RendererFinishedSemaphores;
        std::vector<vk::Fence> m_InFlightFences;

        U32 m_FrameIndex = 0;
    };
} // namespace Rose
