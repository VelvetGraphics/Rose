#include "VulkanViewportImage.hpp"

#include "Platform/Vulkan/Graphics/VulkanAPI.hpp"
#include "Platform/Vulkan/VulkanCall.hpp"

#include <backends/imgui_impl_vulkan.h>

#include "Rose/Renderer/Renderer.hpp"

namespace Rose {
    VulkanViewportImage::VulkanViewportImage()
    {
        std::tie(m_Image, m_ImageMemory) =
                VulkanCall::CreateImage({VulkanAPI::SwapChainExtent().width, VulkanAPI::SwapChainExtent().height, 1},
                                        VulkanAPI::SwapChainSurfaceFormat(), vk::ImageTiling::eOptimal,
                                        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
                                        vk::MemoryPropertyFlagBits::eDeviceLocal, 1);

        m_ImageView = VulkanCall::CreateImageView(m_Image, VulkanAPI::SwapChainSurfaceFormat(), 1,
                                                  vk::ImageAspectFlagBits::eColor);

        VulkanViewportImage::UpdateTexture();
    }

    VulkanViewportImage::~VulkanViewportImage()
    {
        Renderer::WaitDeviceIdle();

        VulkanAPI::Device().destroyImageView(m_ImageView);
        VulkanAPI::Device().destroyImage(m_Image);
        VulkanAPI::Device().freeMemory(m_ImageMemory);

        if (m_ImGuiTexture)
            ImGui_ImplVulkan_RemoveTexture(m_ImGuiTexture);
    }

    void VulkanViewportImage::UpdateTexture()
    {
        if (m_ImGuiTexture)
        {
            ImGui_ImplVulkan_RemoveTexture(m_ImGuiTexture);
        }

        m_ImGuiTexture = ImGui_ImplVulkan_AddTexture(m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    ImTextureID VulkanViewportImage::GetTextureID() const { return reinterpret_cast<ImTextureID>(m_ImGuiTexture); }

    void VulkanViewportImage::Resize(U32 width, U32 height)
    {
        VulkanAPI::Device().destroyImageView(m_ImageView);
        VulkanAPI::Device().destroyImage(m_Image);
        VulkanAPI::Device().freeMemory(m_ImageMemory);

        std::tie(m_Image, m_ImageMemory) = VulkanCall::CreateImage(
                {width, height, 1}, VulkanAPI::SwapChainSurfaceFormat(), vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
                vk::MemoryPropertyFlagBits::eDeviceLocal, 1);

        m_ImageView = VulkanCall::CreateImageView(m_Image, VulkanAPI::SwapChainSurfaceFormat(), 1,
                                                  vk::ImageAspectFlagBits::eColor);

        m_Layout = vk::ImageLayout::eUndefined;

        UpdateTexture();
    }

    ImVec2 VulkanViewportImage::Size()
    {
        return {static_cast<float>(VulkanAPI::SwapChainExtent().width),
                static_cast<float>(VulkanAPI::SwapChainExtent().height)};
    }
} // namespace Rose
