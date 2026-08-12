#include "GraphicsAPI.hpp"

#include "Rose/Core/Core.hpp"
#include "spdlog/fmt/bundled/color.h"

namespace Rose {
    namespace {
        void TransitionImageLayout(vk::CommandBuffer cmdBuf, vk::Image image, vk::ImageLayout oldLayout,
                                   vk::ImageLayout newLayout, vk::AccessFlags2 srcAccessMask,
                                   vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlagBits2 srcStageMask,
                                   vk::PipelineStageFlagBits2 dstStageMask, vk::ImageAspectFlags aspectMask)
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
    } // namespace

    GraphicsAPI::GraphicsAPI(GLFWwindow* window) : m_Window(window) {}

    GraphicsAPI::~GraphicsAPI()
    {
        ASSERT(m_Device.waitIdle() == vk::Result::eSuccess, "Failed to wait for device being idle");

        m_Device.destroyFence(m_InFlightFence);
        m_Device.destroySemaphore(m_ImageAvailableSemaphore);
        for (auto semaphore : m_RendererFinishedSemaphores)
            m_Device.destroySemaphore(semaphore);

        m_Device.freeCommandBuffers(m_CmdPool, 1, &m_CmdBuffer);
        m_Device.destroyCommandPool(m_CmdPool);

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

        return true;
    }

    bool GraphicsAPI::BeginFrame()
    {
        auto drawFinishedResult = m_Device.waitForFences(m_InFlightFence, vk::True, U64Limit);
        if (drawFinishedResult != vk::Result::eSuccess)
            return false;

        vk::CommandBufferBeginInfo beginInfo = {};
        if (m_CmdBuffer.begin(beginInfo) != vk::Result::eSuccess)
            return false;

        vkAcquireNextImageKHR(m_Device, m_SwapChain.swapchain, U64Limit, m_ImageAvailableSemaphore, nullptr,
                              &m_ImageIndex);

        vk::RenderingAttachmentInfo colorAttachment;
        colorAttachment.clearValue = vk::ClearValue({0.0f, 0.0f, 0.0f, 1.0f});
        colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        colorAttachment.imageView = m_SwapChainImageViews[m_ImageIndex];
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;

        vk::RenderingInfo renderingInfo;
        renderingInfo.renderArea = vk::Rect2D{vk::Offset2D{0, 0}, m_SwapChain.extent};
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;

        TransitionImageLayout(m_CmdBuffer, m_SwapChainImages[m_ImageIndex], vk::ImageLayout::eUndefined,
                              vk::ImageLayout::eColorAttachmentOptimal, {}, vk::AccessFlagBits2::eColorAttachmentWrite,
                              vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                              vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::ImageAspectFlagBits::eColor);

        m_CmdBuffer.beginRendering(renderingInfo);

        m_CmdBuffer.setViewport(0, vk::Viewport{0.0f, 0.0f, static_cast<float>(m_SwapChain.extent.width),
                                                static_cast<float>(m_SwapChain.extent.height)});
        m_CmdBuffer.setScissor(0, vk::Rect2D{vk::Offset2D{0, 0}, m_SwapChain.extent});

        ASSERT(m_Device.resetFences(m_InFlightFence) == vk::Result::eSuccess, "Failed to reset fences");

        return true;
    }

    void GraphicsAPI::EndFrame()
    {
        m_CmdBuffer.endRendering();

        TransitionImageLayout(m_CmdBuffer, m_SwapChainImages[m_ImageIndex], vk::ImageLayout::eColorAttachmentOptimal,
                              vk::ImageLayout::ePresentSrcKHR, vk::AccessFlagBits2::eColorAttachmentWrite, {},
                              vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                              vk::PipelineStageFlagBits2::eBottomOfPipe, vk::ImageAspectFlagBits::eColor);

        ASSERT(m_CmdBuffer.end() == vk::Result::eSuccess, "Failed to end command buffer recording");

        vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

        vk::SubmitInfo submitInfo = {};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &m_ImageAvailableSemaphore;
        submitInfo.pWaitDstStageMask = &waitDstStageMask;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_CmdBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_RendererFinishedSemaphores[m_ImageIndex];
        ASSERT(m_GraphicsQueue.submit(submitInfo, m_InFlightFence) == vk::Result::eSuccess,
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
        const vkb::PhysicalDevice& pd = pdResult.value();

        vkb::DeviceBuilder deviceBuilder(pd);
        auto deviceResult = deviceBuilder.build();

        ASSERT(deviceResult, std::string("Failed to create logical device: " + instanceResult.error().message()));
        m_VkbDevice = deviceResult.value();
        m_Device = m_VkbDevice.device;

        auto graphicsQueueResult = m_VkbDevice.get_queue(vkb::QueueType::graphics);
        ASSERT(graphicsQueueResult,
               std::string("Failed to create graphics queue: " + graphicsQueueResult.error().message()));
        m_GraphicsQueue = graphicsQueueResult.value();

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
        ASSERT(cmdPoolResult.result == vk::Result::eSuccess, std::string("Failed to create command pool"));
        m_CmdPool = cmdPoolResult.value;

        vk::CommandBufferAllocateInfo allocInfo = {};
        allocInfo.commandPool = m_CmdPool;
        allocInfo.commandBufferCount = 1;
        allocInfo.level = vk::CommandBufferLevel::ePrimary;

        auto cmdBufferResult = m_Device.allocateCommandBuffers(allocInfo);
        ASSERT(cmdBufferResult.result == vk::Result::eSuccess, "Failed to allocate command buffer");
        m_CmdBuffer = cmdBufferResult.value[0];
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

        auto semaphoreResult = m_Device.createSemaphore(semaphoreInfo);
        ASSERT(semaphoreResult.result == vk::Result::eSuccess, "Failed to create semaphore");

        m_ImageAvailableSemaphore = semaphoreResult.value;

        vk::FenceCreateInfo fenceInfo = {};
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
        auto fenceResult = m_Device.createFence(fenceInfo);
        ASSERT(fenceResult.result == vk::Result::eSuccess, "Failed to create fence");

        m_InFlightFence = fenceResult.value;
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
