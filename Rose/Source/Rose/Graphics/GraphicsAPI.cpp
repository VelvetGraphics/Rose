#include "GraphicsAPI.hpp"

#include "Rose/Core/Core.hpp"

namespace Rose {
    GraphicsAPI::GraphicsAPI(GLFWwindow* window) : m_Window(window) {}

    GraphicsAPI::~GraphicsAPI()
    {
        m_Device.freeCommandBuffers(m_CmdPool, 1, &m_CmdBuffer);
        m_Device.destroyCommandPool(m_CmdPool);

        vkb::destroy_swapchain(m_SwapChain);
        vkb::destroy_device(m_VkbDevice);
        vkb::destroy_surface(m_Instance, m_Surface);
        vkb::destroy_instance(m_Instance);
    }

    bool GraphicsAPI::Init()
    {
        BootStrap();
        CreateCmdBuffers();
        return true;
    }

    bool GraphicsAPI::BeginFrame() { return true; }

    void GraphicsAPI::EndFrame() {}

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

        vkb::PhysicalDeviceSelector physicalDeviceSelector(m_Instance);
        auto pdResult = physicalDeviceSelector.set_surface(m_Surface)
                                .set_minimum_version(1, 1)
                                .require_dedicated_transfer_queue()
                                .select();

        ASSERT(pdResult, std::string("Failed to select physical device: " + pdResult.error().message()));

        vkb::DeviceBuilder deviceBuilder(pdResult.value());
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
    }

    void GraphicsAPI::CreateCmdBuffers()
    {
        vk::CommandPoolCreateInfo poolInfo = {};
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        poolInfo.queueFamilyIndex = m_VkbDevice.get_queue_index(vkb::QueueType::graphics).value();

        auto cmdPoolResult = m_Device.createCommandPool(poolInfo);
        ASSERT((cmdPoolResult.result == vk::Result::eSuccess), std::string("Failed to create command pool"));
        m_CmdPool = cmdPoolResult.value;

        vk::CommandBufferAllocateInfo allocInfo = {};
        allocInfo.commandPool = m_CmdPool;
        allocInfo.commandBufferCount = 1;
        allocInfo.level = vk::CommandBufferLevel::ePrimary;

        auto cmdBufferResult = m_Device.allocateCommandBuffers(allocInfo);
    }
} // namespace Rose
