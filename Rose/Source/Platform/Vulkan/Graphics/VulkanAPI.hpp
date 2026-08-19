#pragma once
#include "Platform/Vulkan/Graphics/VulkanViewportImage.hpp"
#include "Platform/Vulkan/VulkanInclude.hpp"
#include "Rose/Base/EventTypes.hpp"
#include "Rose/Graphics/GraphicsAPI.hpp"

namespace Rose {
    enum class QueueType : bool
    {
        Graphics = false,
        Transfer = true
    };

    class VulkanAPI : public GraphicsAPI
    {
    public:
        explicit VulkanAPI(GLFWwindow* window);
        ~VulkanAPI() override;

        bool Init() override;

        bool BeginFrame() override;
        void EndFrame() override;

        void MakeContextCurrent() override { s_CurrentContext = this; }
        void WaitDeviceIdle() const override;

        void SetViewportImage(ViewportImage& viewport) override;

        static vk::Instance Instance() { return s_CurrentContext->m_Instance.instance; }

        static vk::PhysicalDevice PhysicalDevice() { return s_CurrentContext->m_PhysicalDevice.physical_device; }
        static vk::Device Device() { return s_CurrentContext->m_Device; }

        static vk::CommandPool CmdPool() { return s_CurrentContext->m_GraphicsCmdPool; }
        static U32 FrameIndex() { return s_CurrentContext->m_FrameIndex; }

        static U32 GraphicsQueueFamilyIndex();
        static vk::Queue GraphicsQueue() { return s_CurrentContext->m_GraphicsQueue; }

        static U32 SwapChainImageCount() { return s_CurrentContext->m_SwapChainImages.size(); }
        static vk::Format SwapChainSurfaceFormat();
        static vk::Extent2D SwapChainExtent() { return s_CurrentContext->m_SwapChain.extent; }

        static vk::Format DepthFormat() { return s_CurrentContext->m_DepthFormat; }

        static void Submit(std::function<void(vk::CommandBuffer)> cmd);
        static void SubmitSingleTime(const std::function<void(vk::CommandBuffer)>& cmd, QueueType type);

        static constexpr U32 MaxFramesInFlight() { return s_MaxFramesInFlight; }

        static vk::PipelineCache PipelineCache() { return s_CurrentContext->m_PipelineCache; }

    private:
        void BootStrap();
        void CreateCmdBuffers();
        void CreateSyncObjects();
        void CreateDepthResources();
        void CreatePipelineCache();

        void HandleSwapChainResize();
        void RecreateSwapChain();

        void ExecCommands();

        static vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
                                              vk::FormatFeatureFlags features);

        void OnWindowResize(const WindowResizedEvent& e);

        void SavePipelineCache() const;
        static std::vector<U8> LoadPipelineCache();

    private:
        inline static VulkanAPI* s_CurrentContext = nullptr;

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
        static constexpr U32 s_MaxFramesInFlight = 1;

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

        vk::PipelineCache m_PipelineCache;
        static constexpr const char* s_PipelineCachePath = ".Cache/Vulkan/PipelineCache.bin";

        VulkanViewportImage* m_Viewport = nullptr;
    };
} // namespace Rose
