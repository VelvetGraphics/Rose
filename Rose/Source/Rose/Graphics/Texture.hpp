#pragma once

#include "Rose/Graphics/VulkanInclude.hpp"

namespace Rose {
    enum class TextureFilter : U8
    {
        Linear,
        Nearest
    };

    enum class TextureWrap : U8
    {
        Repeat,
        MirroredRepeat,
        ClampToEdge,
        MirroredClampToEdge,
    };

    struct TextureSamplerInfo
    {
        TextureFilter MinFilter = TextureFilter::Linear;
        TextureFilter MagFilter = TextureFilter::Linear;

        TextureWrap WrapU = TextureWrap::Repeat;
        TextureWrap WrapV = TextureWrap::Repeat;
        TextureWrap WrapW = TextureWrap::Repeat;

        TextureFilter MipMapMode = TextureFilter::Linear;
    };

    class Texture : public RefCounted
    {
    public:
        ~Texture() override;
        static Ref<Texture> Create(std::string path, TextureSamplerInfo samplerInfo);

        void Reload();

    private:
        void UnLoad();

        Texture(std::string&& path, TextureSamplerInfo samplerInfo);

        void CreateImage();
        void CreateSampler();

    private:
        std::string m_Path;

        TextureSamplerInfo m_SamplerInfo;
        vk::Sampler m_Sampler;

        vk::Image m_Image;
        vk::DeviceMemory m_ImageMemory;
        vk::ImageView m_ImageView;

        bool m_Loaded = false;

        friend class Shader;
        friend class Ref<Texture>;
    };
} // namespace Rose
