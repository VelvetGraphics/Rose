#pragma once
#include "Rose/Graphics/ViewportImage.hpp"

#include "Platform/Vulkan/VulkanInclude.hpp"

namespace Rose {
    class VulkanViewportImage : public ViewportImage
    {
    public:
        VulkanViewportImage();
        ~VulkanViewportImage() override;

        void UpdateTexture() override;
        ImTextureID GetTextureID() const override;

        void Resize(U32 width, U32 height) override;
        ImVec2 Size() override;

    private:
        vk::Image m_Image;
        vk::DeviceMemory m_ImageMemory;
        vk::ImageView m_ImageView;
        vk::ImageLayout m_Layout = vk::ImageLayout::eUndefined;

        VkDescriptorSet m_ImGuiTexture = nullptr;

        friend class VulkanAPI;
    };
} // namespace Rose
