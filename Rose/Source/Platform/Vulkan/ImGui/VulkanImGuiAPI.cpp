#include "VulkanImGuiAPI.hpp"

#include "Platform/Vulkan/Graphics/VulkanAPI.hpp"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

namespace Rose {
    void VulkanImGuiAPI::Init(GLFWwindow* window)
    {
        vk::Format surfaceFormat = VulkanAPI::SwapChainSurfaceFormat();
        vk::PipelineRenderingCreateInfo pipelineRenderingInfo = {};
        pipelineRenderingInfo.colorAttachmentCount = 1;
        pipelineRenderingInfo.pColorAttachmentFormats = &surfaceFormat;
        pipelineRenderingInfo.depthAttachmentFormat = VulkanAPI::DepthFormat();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();

        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = VulkanAPI::Instance();
        initInfo.PhysicalDevice = VulkanAPI::PhysicalDevice();
        initInfo.Device = VulkanAPI::Device();
        initInfo.QueueFamily = VulkanAPI::GraphicsQueueFamilyIndex();
        initInfo.Queue = VulkanAPI::GraphicsQueue();
        initInfo.PipelineCache = VulkanAPI::PipelineCache();
        initInfo.DescriptorPoolSize = 16;
        initInfo.MinImageCount = VulkanAPI::SwapChainImageCount();
        initInfo.ImageCount = VulkanAPI::SwapChainImageCount();
        initInfo.Allocator = nullptr;
        initInfo.PipelineInfoMain.RenderPass = nullptr;
        initInfo.UseDynamicRendering = true;
        initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineRenderingInfo;
        ImGui_ImplVulkan_Init(&initInfo);
    }

    void VulkanImGuiAPI::Shutdown()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void VulkanImGuiAPI::BeginFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void VulkanImGuiAPI::EndFrame() { ImGui::Render(); }
} // namespace Rose
