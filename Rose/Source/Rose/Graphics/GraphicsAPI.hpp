#pragma once
#include "Rose/Base/EventTypes.hpp"
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
        void WaitDeviceIdle() const;

        static vk::PhysicalDevice PhysicalDevice() { return s_CurrentContext->m_PhysicalDevice.physical_device; }
        static vk::Device Device() { return s_CurrentContext->m_Device; }

        static vk::CommandPool CmdPool() { return s_CurrentContext->m_GraphicsCmdPool; }

        static vk::Format SwapChainSurfaceFormat();
        static vk::Extent2D SwapChainExtent() { return s_CurrentContext->m_SwapChain.extent; }

        static vk::Format DepthFormat() { return s_CurrentContext->m_DepthFormat; }

        static void Submit(std::function<void(vk::CommandBuffer)> cmd);
        static void SubmitSingleTime(const std::function<void(vk::CommandBuffer)>& cmd);

    private:
        void BootStrap();
        void CreateCmdBuffers();
        void CreateSyncObjects();
        void CreateDepthResources();

        void RecreateSwapChain();

        void ExecCommands();

        static vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
                                              vk::FormatFeatureFlags features);

        void OnWindowResize(WindowResizedEvent& e);

    private:
        inline static GraphicsAPI* s_CurrentContext = nullptr;

        GLFWwindow* m_Window = nullptr;
        vk::SurfaceKHR m_Surface;

        vkb::Instance m_Instance;

        vkb::PhysicalDevice m_PhysicalDevice;

        vkb::Device m_VkbDevice;
        vk::Device m_Device;

        vk::Queue m_GraphicsQueue;
        vk::Queue m_TransferQueue;

        vkb::Swapchain m_SwapChain;
        std::vector<vk::Image> m_SwapChainImages;
        std::vector<vk::ImageView> m_SwapChainImageViews;
        U32 m_ImageIndex = 0;

        vk::CommandPool m_GraphicsCmdPool;
        vk::CommandPool m_TransferCmdPool;
        std::vector<vk::CommandBuffer> m_CmdBuffers;
        std::vector<std::function<void(vk::CommandBuffer)>> m_Commands;

        std::vector<vk::Semaphore> m_ImageAvailableSemaphores;
        std::vector<vk::Semaphore> m_RendererFinishedSemaphores;
        std::vector<vk::Fence> m_InFlightFences;
        U32 m_FrameIndex = 0;

        vk::Image m_DepthImage;
        vk::DeviceMemory m_DepthImageMemory;
        vk::ImageView m_DepthImageView;
        vk::Format m_DepthFormat = vk::Format::eUndefined;
    };
} // namespace Rose
