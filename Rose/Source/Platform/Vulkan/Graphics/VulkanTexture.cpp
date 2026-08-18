#include "VulkanTexture.hpp"

#include "Platform/Vulkan/Graphics/VulkanAPI.hpp"
#include "Platform/Vulkan/VulkanCall.hpp"
#include "Rose/Core/Core.hpp"

#include <stb_image/stb_image.h>

namespace Rose {
    namespace {
        struct Image
        {
            U32 Width = 0, Height = 0, Channels = 0;
            U8* Data = nullptr;

            ~Image() { stbi_image_free(Data); }
        };

        Image LoadImage(const std::string& path)
        {
            Image img;
            int width, height, channels;
            img.Data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

            img.Width = width;
            img.Height = height;
            img.Channels = channels;

            ASSERT(img.Data, "Failed to load image");

            return img;
        }

        vk::Filter TranslateTextureFilter(TextureFilter filter)
        {
            switch (filter)
            {
                case TextureFilter::Linear:
                    return vk::Filter::eLinear;
                case TextureFilter::Nearest:
                    return vk::Filter::eNearest;
            }

            ASSERT(false, "Invalid texture filter");
        }

        vk::SamplerAddressMode TranslateTextureWrap(TextureWrap wrap)
        {
            switch (wrap)
            {
                case TextureWrap::Repeat:
                    return vk::SamplerAddressMode::eRepeat;
                case TextureWrap::MirroredRepeat:
                    return vk::SamplerAddressMode::eMirroredRepeat;
                case TextureWrap::ClampToEdge:
                    return vk::SamplerAddressMode::eClampToEdge;
                case TextureWrap::MirroredClampToEdge:
                    return vk::SamplerAddressMode::eMirrorClampToEdge;
            }

            ASSERT(false, "Invalid texture wrap");
        }
    } // namespace

    VulkanTexture::VulkanTexture(std::string&& path, TextureSamplerInfo samplerInfo) :
        Texture(std::move(path)), m_SamplerInfo(samplerInfo)
    {
        VulkanTexture::Reload();
    }

    VulkanTexture::~VulkanTexture()
    {
        if (m_Loaded)
            UnLoad();
    }

    void VulkanTexture::Reload()
    {
        if (m_Loaded)
            UnLoad();

        CreateImage();
        m_ImageView = VulkanCall::CreateImageView(m_Image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
        CreateSampler();

        m_Loaded = true;
    }

    void VulkanTexture::UnLoad()
    {
        VulkanAPI::Device().destroySampler(m_Sampler);

        VulkanAPI::Device().destroyImageView(m_ImageView);
        VulkanAPI::Device().destroyImage(m_Image);
        VulkanAPI::Device().freeMemory(m_ImageMemory);

        m_Loaded = false;
    }

    void VulkanTexture::CreateImage()
    {
        Image image = LoadImage(m_Path);
        vk::DeviceSize imageSize = image.Width * image.Height * 4;

        auto [stagingBuffer, stagingBufferMemory] = VulkanCall::CreateBuffer(
                imageSize, vk::BufferUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        void* location = VulkanAPI::Device().mapMemory(stagingBufferMemory, 0, imageSize).value;
        memcpy(location, image.Data, imageSize);
        VulkanAPI::Device().unmapMemory(stagingBufferMemory);

        std::tie(m_Image, m_ImageMemory) = VulkanCall::CreateImage(
                vk::Extent3D{(image.Width), (image.Height), 1}, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
                vk::MemoryPropertyFlagBits::eDeviceLocal);

        VulkanAPI::SubmitSingleTime(
                [&](vk::CommandBuffer cmdBuffer) {
                    VulkanCall::TransitionImageLayout(cmdBuffer, m_Image, vk::ImageLayout::eUndefined,
                                                      vk::ImageLayout::eTransferDstOptimal,
                                                      vk::ImageAspectFlagBits::eColor);

                    VulkanCall::CopyBufferToImage(cmdBuffer, stagingBuffer, m_Image,
                                                  vk::Extent3D{image.Width, image.Height, 1},
                                                  vk::ImageAspectFlagBits::eColor);

                    VulkanCall::TransitionImageLayout(cmdBuffer, m_Image, vk::ImageLayout::eTransferDstOptimal,
                                                      vk::ImageLayout::eShaderReadOnlyOptimal,
                                                      vk::ImageAspectFlagBits::eColor);
                },
                QueueType::Graphics);

        VulkanAPI::Device().destroyBuffer(stagingBuffer);
        VulkanAPI::Device().freeMemory(stagingBufferMemory);
    }

    void VulkanTexture::CreateSampler()
    {
        vk::PhysicalDeviceProperties properties = VulkanAPI::PhysicalDevice().getProperties();

        vk::SamplerCreateInfo samplerInfo = {};
        samplerInfo.minFilter = TranslateTextureFilter(m_SamplerInfo.MinFilter);
        samplerInfo.magFilter = TranslateTextureFilter(m_SamplerInfo.MagFilter);
        samplerInfo.addressModeU = TranslateTextureWrap(m_SamplerInfo.WrapU);
        samplerInfo.addressModeV = TranslateTextureWrap(m_SamplerInfo.WrapV);
        samplerInfo.addressModeW = TranslateTextureWrap(m_SamplerInfo.WrapW);

        samplerInfo.mipmapMode = m_SamplerInfo.MipMapMode == TextureFilter::Linear ? vk::SamplerMipmapMode::eLinear
                                                                                   : vk::SamplerMipmapMode::eNearest;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        samplerInfo.anisotropyEnable = vk::True;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.compareEnable = vk::False;
        samplerInfo.compareOp = vk::CompareOp::eAlways;
        samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        samplerInfo.unnormalizedCoordinates = vk::False;

        auto samplerResult = VulkanAPI::Device().createSampler(samplerInfo);
        ASSERT(samplerResult.result == vk::Result::eSuccess, "Failed to create sampler");
        m_Sampler = samplerResult.value;
    }
} // namespace Rose
