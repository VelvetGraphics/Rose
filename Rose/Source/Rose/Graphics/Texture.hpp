#pragma once

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
        ~Texture() override = default;

        static Ref<Texture> Create(std::string path, TextureSamplerInfo samplerInfo);

        virtual void Reload() = 0;

        const std::string& GetPath() { return m_Path; }
        void SetPath(std::string path) { m_Path = std::move(path); }

    protected:
        Texture(std::string&& path) : m_Path(path) {}

    protected:
        std::string m_Path;
    };
} // namespace Rose
