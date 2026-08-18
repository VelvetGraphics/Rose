#pragma once
#include "Rose/Graphics/Texture.hpp"

#include "Platform/Vulkan/VulkanInclude.hpp"

namespace Rose {
    class VulkanTexture : public Texture
    {
    public:
        VulkanTexture(std::string&& path, TextureSamplerInfo samplerInfo);
        ~VulkanTexture() override;

        void Reload() override;

    private:
        void UnLoad();

        void CreateImage();
        void GenerateMipMaps();
        void CreateSampler();

    private:
        TextureSamplerInfo m_SamplerInfo;
        vk::Sampler m_Sampler;

        vk::Image m_Image;
        vk::DeviceMemory m_ImageMemory;
        vk::ImageView m_ImageView;

        U32 m_MipLevels = 0;
        S32 m_MipWidth = 0;
        S32 m_MipHeight = 0;

        bool m_Loaded = false;

        friend class VulkanShader;
    };
} // namespace Rose
