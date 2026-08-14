#include "GraphicsAPI.hpp"

#include "Rose/Core/Core.hpp"
#include "Rose/Graphics/VulkanCall.hpp"
#include "Rose/Main/Main.hpp"

namespace Rose {
    namespace {
        void TransitionImageLayout(vk::CommandBuffer cmdBuf, vk::Image image, vk::ImageLayout oldLayout,
                                   vk::ImageLayout newLayout, vk::AccessFlags2 srcAccessMask,
                                   vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 srcStageMask,
                                   vk::PipelineStageFlags2 dstStageMask, vk::ImageAspectFlags aspectMask)
        {
            vk::ImageMemoryBarrier2 barrier = {};
            barrier.image = image;
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.srcAccessMask = srcAccessMask;
            barrier.dstAccessMask = dstAccessMask;
            barrier.srcStageMask = srcStageMask;
            barrier.dstStageMask = dstStageMask;
            barrier.subresourceRange.aspectMask = aspectMask;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseMipLevel = 0;

            vk::DependencyInfo dependencyInfo = {};
            dependencyInfo.dependencyFlags = {};
            dependencyInfo.imageMemoryBarrierCount = 1;
            dependencyInfo.pImageMemoryBarriers = &barrier;

            cmdBuf.pipelineBarrier2(dependencyInfo);
        }

        constexpr U32 g_MaxFramesInFlight = 2;
    } // namespace

    GraphicsAPI::GraphicsAPI(GLFWwindow* window) : m_Window(window) {}

    GraphicsAPI::~GraphicsAPI()
    {
        m_Device.destroyImageView(m_DepthImageView);
        m_Device.freeMemory(m_DepthImageMemory);
        m_Device.destroyImage(m_DepthImage);

        for (auto fence : m_InFlightFences)
            m_Device.destroyFence(fence);

        for (auto semaphore : m_ImageAvailableSemaphores)
            m_Device.destroySemaphore(semaphore);

        for (auto semaphore : m_RendererFinishedSemaphores)
            m_Device.destroySemaphore(semaphore);

        m_Device.destroyCommandPool(m_TransferCmdPool);

        m_Device.freeCommandBuffers(m_GraphicsCmdPool, m_CmdBuffers.size(), m_CmdBuffers.data());
        m_Device.destroyCommandPool(m_GraphicsCmdPool);

        for (auto imageView : m_SwapChainImageViews)
            m_Device.destroyImageView(imageView);

        vkb::destroy_swapchain(m_SwapChain);
        vkb::destroy_device(m_VkbDevice);
        vkb::destroy_surface(m_Instance, m_Surface);
        vkb::destroy_instance(m_Instance);
    }

    bool GraphicsAPI::Init()
    {
        BootStrap();
        CreateCmdBuffers();
        CreateSyncObjects();
        CreateDepthResources();

        Main::GetEventBus().Observe<WindowResizedEvent>(
                [this](WindowResizedEvent& e) -> decltype(auto) { OnWindowResize(e); });

        return true;
    }

    bool GraphicsAPI::BeginFrame()
    {
        m_FrameIndex = (m_FrameIndex + 1) % g_MaxFramesInFlight;

        auto drawFinishedResult = m_Device.waitForFences(m_InFlightFences[m_FrameIndex], vk::True, U64Limit);
        if (drawFinishedResult != vk::Result::eSuccess)
            return false;

        vk::CommandBufferBeginInfo beginInfo = {};
        if (m_CmdBuffers[m_FrameIndex].begin(beginInfo) != vk::Result::eSuccess)
            return false;

        vkAcquireNextImageKHR(m_Device, m_SwapChain.swapchain, U64Limit, m_ImageAvailableSemaphores[m_FrameIndex],
                              nullptr, &m_ImageIndex);

        vk::RenderingAttachmentInfo colorAttachment;
        colorAttachment.clearValue = vk::ClearValue({0.0f, 0.0f, 0.0f, 1.0f});
        colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        colorAttachment.imageView = m_SwapChainImageViews[m_ImageIndex];
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;

        vk::ClearValue depthClear = vk::ClearDepthStencilValue(1.0f, 0.0f);
        vk::RenderingAttachmentInfo depthAttachment;
        depthAttachment.clearValue = depthClear;
        depthAttachment.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
        depthAttachment.imageView = m_DepthImageView;
        depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;

        vk::RenderingInfo renderingInfo;
        renderingInfo.renderArea = vk::Rect2D{vk::Offset2D{0, 0}, m_SwapChain.extent};
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = &depthAttachment;

        TransitionImageLayout(m_CmdBuffers[m_FrameIndex], m_SwapChainImages[m_ImageIndex], vk::ImageLayout::eUndefined,
                              vk::ImageLayout::eColorAttachmentOptimal, {}, vk::AccessFlagBits2::eColorAttachmentWrite,
                              vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                              vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::ImageAspectFlagBits::eColor);

        TransitionImageLayout(
                m_CmdBuffers[m_FrameIndex], m_DepthImage, vk::ImageLayout::eUndefined,
                vk::ImageLayout::eDepthAttachmentOptimal, vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
                vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
                vk::ImageAspectFlagBits::eDepth);

        m_CmdBuffers[m_FrameIndex].beginRendering(renderingInfo);

        m_CmdBuffers[m_FrameIndex].setViewport(0, vk::Viewport{0.0f, 0.0f, static_cast<float>(m_SwapChain.extent.width),
                                                               static_cast<float>(m_SwapChain.extent.height)});
        m_CmdBuffers[m_FrameIndex].setScissor(0, vk::Rect2D{vk::Offset2D{0, 0}, m_SwapChain.extent});

        ASSERT(m_Device.resetFences(m_InFlightFences[m_FrameIndex]) == vk::Result::eSuccess, "Failed to reset fences");

        return true;
    }

    void GraphicsAPI::EndFrame()
    {
        ExecCommands();
        m_CmdBuffers[m_FrameIndex].endRendering();

        TransitionImageLayout(m_CmdBuffers[m_FrameIndex], m_SwapChainImages[m_ImageIndex],
                              vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
                              vk::AccessFlagBits2::eColorAttachmentWrite, {},
                              vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                              vk::PipelineStageFlagBits2::eBottomOfPipe, vk::ImageAspectFlagBits::eColor);

        ASSERT(m_CmdBuffers[m_FrameIndex].end() == vk::Result::eSuccess, "Failed to end command buffer recording");

        vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

        vk::SubmitInfo submitInfo = {};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &m_ImageAvailableSemaphores[m_FrameIndex];
        submitInfo.pWaitDstStageMask = &waitDstStageMask;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_CmdBuffers[m_FrameIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_RendererFinishedSemaphores[m_ImageIndex];
        ASSERT(m_GraphicsQueue.submit(submitInfo, m_InFlightFences[m_FrameIndex]) == vk::Result::eSuccess,
               "Failed to submit command buffer to graphics queue");

        vk::SwapchainKHR swapChain = m_SwapChain.swapchain;

        vk::PresentInfoKHR presentInfo = {};
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_RendererFinishedSemaphores[m_ImageIndex];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapChain;
        presentInfo.pImageIndices = &m_ImageIndex;

        vk::Result presentResult = m_GraphicsQueue.presentKHR(presentInfo);
        if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR)
        {
            RecreateSwapChain();
        }
    }

    void GraphicsAPI::WaitDeviceIdle() const
    {
        ASSERT(m_Device.waitIdle() == vk::Result::eSuccess, "Failed to wait for device idle");
    }

    vk::Format GraphicsAPI::SwapChainSurfaceFormat()
    {
        return static_cast<vk::Format>(s_CurrentContext->m_SwapChain.image_format);
    }

    void GraphicsAPI::Submit(std::function<void(vk::CommandBuffer)> cmd)
    {
        s_CurrentContext->m_Commands.emplace_back(std::move(cmd));
    }

    void GraphicsAPI::SubmitSingleTime(const std::function<void(vk::CommandBuffer)>& cmd)
    {
        vk::CommandBufferAllocateInfo allocInfo = {};
        allocInfo.commandPool = s_CurrentContext->m_TransferCmdPool;
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandBufferCount = 1;

        vk::CommandBuffer cmdBuffer = Device().allocateCommandBuffers(allocInfo).value[0];

        vk::CommandBufferBeginInfo begin = {};
        begin.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        ASSERT(cmdBuffer.begin(begin) == vk::Result::eSuccess, "Failed to begin single time command buffer");

        cmd(cmdBuffer);

        ASSERT(cmdBuffer.end() == vk::Result::eSuccess, "Failed to end single time command buffer");
        vk::SubmitInfo submitInfo = {};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;
        ASSERT(s_CurrentContext->m_TransferQueue.submit(submitInfo) == vk::Result::eSuccess,
               "Failed to submit single time commands to queue");

        ASSERT(s_CurrentContext->m_TransferQueue.waitIdle() == vk::Result::eSuccess, "Failed to wait for queue idle");
        Device().freeCommandBuffers(s_CurrentContext->m_TransferCmdPool, cmdBuffer);
    }

    void GraphicsAPI::ExecCommands()
    {
        for (auto cmd : m_Commands)
            cmd(m_CmdBuffers[m_FrameIndex]);

        m_Commands.clear();
    }

    vk::Format GraphicsAPI::FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
                                                vk::FormatFeatureFlags features)
    {
        for (vk::Format format : candidates)
        {
            vk::FormatProperties props = GraphicsAPI::PhysicalDevice().getFormatProperties(format);

            if ((tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) ||
                (tiling == vk::ImageTiling::eOptimal) && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        ASSERT(false, "Failed to find suitable image format");
    }

    void GraphicsAPI::OnWindowResize(WindowResizedEvent& e)
    {
        if (e.GetWidth() == 0 || e.GetHeight() == 0)
            return;

        RecreateSwapChain();

        m_Device.destroyImageView(m_DepthImageView);
        m_Device.freeMemory(m_DepthImageMemory);
        m_Device.destroyImage(m_DepthImage);

        CreateDepthResources();
    }

    void GraphicsAPI::BootStrap()
    {
        vkb::InstanceBuilder instanceBuilder;
        auto instanceResult = instanceBuilder.set_app_name("Rose")
                                      .require_api_version(1, 3)
                                      .request_validation_layers()
                                      .use_default_debug_messenger()
                                      .build();

        ASSERT(instanceResult, std::string("Failed to create instance: " + instanceResult.error().message()));
        m_Instance = instanceResult.value();

        VkSurfaceKHR rawSurface;
        glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &rawSurface);
        m_Surface = rawSurface;

        VkPhysicalDeviceVulkan13Features features13 = {};
        features13.synchronization2 = vk::True;
        features13.dynamicRendering = vk::True;

        vkb::PhysicalDeviceSelector physicalDeviceSelector(m_Instance);
        auto pdResult = physicalDeviceSelector.set_surface(m_Surface)
                                .set_minimum_version(1, 1)
                                .set_required_features_13(features13)
                                .require_dedicated_transfer_queue()
                                .select();

        ASSERT(pdResult, std::string("Failed to select physical device: " + pdResult.error().message()));
        m_PhysicalDevice = pdResult.value();

        vkb::DeviceBuilder deviceBuilder(m_PhysicalDevice);
        auto deviceResult = deviceBuilder.build();

        ASSERT(deviceResult, std::string("Failed to create logical device: " + instanceResult.error().message()));
        m_VkbDevice = deviceResult.value();
        m_Device = m_VkbDevice.device;

        auto graphicsQueueResult = m_VkbDevice.get_queue(vkb::QueueType::graphics);
        ASSERT(graphicsQueueResult,
               std::string("Failed to create graphics queue: " + graphicsQueueResult.error().message()));
        m_GraphicsQueue = graphicsQueueResult.value();

        auto transferQueueResult = m_VkbDevice.get_dedicated_queue(vkb::QueueType::transfer);
        ASSERT(transferQueueResult,
               std::string("Failed to create graphics queue: " + transferQueueResult.error().message()));
        m_TransferQueue = transferQueueResult.value();

        vkb::SwapchainBuilder swapChainBuilder(m_VkbDevice);
        auto swapChainResult = swapChainBuilder.build();

        ASSERT(swapChainResult, std::string("Failed to create swap chain: " + swapChainResult.error().message()));
        m_SwapChain = swapChainResult.value();

        std::vector<VkImage> rawImages = m_SwapChain.get_images().value();
        for (VkImage image : rawImages)
            m_SwapChainImages.emplace_back(image);

        std::vector<VkImageView> rawImageViews = m_SwapChain.get_image_views().value();
        for (VkImageView imageView : rawImageViews)
            m_SwapChainImageViews.emplace_back(imageView);
    }

    void GraphicsAPI::CreateCmdBuffers()
    {
        vk::CommandPoolCreateInfo poolInfo = {};
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        poolInfo.queueFamilyIndex = m_VkbDevice.get_queue_index(vkb::QueueType::graphics).value();

        auto cmdPoolResult = m_Device.createCommandPool(poolInfo);
        ASSERT(cmdPoolResult.result == vk::Result::eSuccess, std::string("Failed to create graphics command pool"));
        m_GraphicsCmdPool = cmdPoolResult.value;

        vk::CommandBufferAllocateInfo allocInfo = {};
        allocInfo.commandPool = m_GraphicsCmdPool;
        allocInfo.commandBufferCount = g_MaxFramesInFlight;
        allocInfo.level = vk::CommandBufferLevel::ePrimary;

        auto cmdBufferResult = m_Device.allocateCommandBuffers(allocInfo);
        ASSERT(cmdBufferResult.result == vk::Result::eSuccess, "Failed to allocate command buffer");
        m_CmdBuffers = cmdBufferResult.value;

        poolInfo.queueFamilyIndex = m_VkbDevice.get_queue_index(vkb::QueueType::transfer).value();
        cmdPoolResult = m_Device.createCommandPool(poolInfo);
        ASSERT(cmdPoolResult.result == vk::Result::eSuccess, std::string("Failed to create transfer command pool"));
        m_TransferCmdPool = cmdPoolResult.value;
    }

    void GraphicsAPI::CreateSyncObjects()
    {
        vk::SemaphoreCreateInfo semaphoreInfo = {};
        for (U32 i = 0; i < m_SwapChain.image_count; i++)
        {
            auto semaphoreResult = m_Device.createSemaphore(semaphoreInfo);
            ASSERT(semaphoreResult.result == vk::Result::eSuccess, "Failed to create semaphore");

            m_RendererFinishedSemaphores.push_back(semaphoreResult.value);
        }

        vk::FenceCreateInfo fenceInfo = {};
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        for (U32 i = 0; i < g_MaxFramesInFlight; i++)
        {
            auto semaphoreResult = m_Device.createSemaphore(semaphoreInfo);
            ASSERT(semaphoreResult.result == vk::Result::eSuccess, "Failed to create semaphore");

            m_ImageAvailableSemaphores.push_back(semaphoreResult.value);

            auto fenceResult = m_Device.createFence(fenceInfo);
            ASSERT(fenceResult.result == vk::Result::eSuccess, "Failed to create fence");

            m_InFlightFences.push_back(fenceResult.value);
        }
    }

    void GraphicsAPI::CreateDepthResources()
    {
        m_DepthFormat =
                FindSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                                    vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

        std::tie(m_DepthImage, m_DepthImageMemory) = VulkanCall::CreateImage(
                vk::Extent3D{m_SwapChain.extent.width, m_SwapChain.extent.height, 1}, m_DepthFormat,
                vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
                vk::MemoryPropertyFlagBits::eDeviceLocal);

        m_DepthImageView = VulkanCall::CreateImageView(m_DepthImage, m_DepthFormat, vk::ImageAspectFlagBits::eDepth);
    }

    void GraphicsAPI::RecreateSwapChain()
    {
        int width, height;
        glfwGetFramebufferSize(m_Window, &width, &height);
        if (width == 0 || height == 0)
            return;

        ASSERT(m_Device.waitIdle() == vk::Result::eSuccess, "Failed to wait for device idle");

        for (auto imageView : m_SwapChainImageViews)
            m_Device.destroyImageView(imageView);

        m_SwapChainImages.clear();
        m_SwapChainImageViews.clear();

        for (auto& semaphore : m_RendererFinishedSemaphores)
            m_Device.destroySemaphore(semaphore);
        m_RendererFinishedSemaphores.clear();

        vkb::Swapchain oldSwapChain = m_SwapChain;

        vkb::SwapchainBuilder builder(m_VkbDevice);
        auto swapChainResult = builder.set_old_swapchain(m_SwapChain).set_desired_extent(width, height).build();

        ASSERT(swapChainResult, std::string("Failed to create swap chain: " + swapChainResult.error().message()));
        m_SwapChain = swapChainResult.value();

        vkb::destroy_swapchain(oldSwapChain);

        std::vector<VkImage> rawImages = m_SwapChain.get_images().value();
        for (VkImage image : rawImages)
            m_SwapChainImages.emplace_back(image);

        std::vector<VkImageView> rawImageViews = m_SwapChain.get_image_views().value();
        for (VkImageView imageView : rawImageViews)
            m_SwapChainImageViews.emplace_back(imageView);

        vk::SemaphoreCreateInfo semaphoreInfo{};

        m_RendererFinishedSemaphores.resize(m_SwapChainImages.size());
        for (auto& semaphore : m_RendererFinishedSemaphores)
            semaphore = m_Device.createSemaphore(semaphoreInfo).value;
    }
} // namespace Rose
