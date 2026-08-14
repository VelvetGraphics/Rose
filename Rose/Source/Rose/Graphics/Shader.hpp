#pragma once

#include "Rose/Graphics/VulkanInclude.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_CXX20
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace Rose {
    enum class ShaderStage : U8
    {
        Vertex,
        Fragment
    };

    struct ShaderSources
    {
        std::string VertexShader;
        std::string FragmentShader;
    };

    enum class ShaderDataType
    {
        None,

        Bool,

        Int,
        Int2,
        Int3,
        Int4,

        Float,
        Float2,
        Float3,
        Float4,

        Mat2,
        Mat3,
        Mat4
    };

    struct VertexShaderAttribute
    {
        U32 Location = -1;
        ShaderDataType DataType = ShaderDataType::None;

        VertexShaderAttribute(U32 location, ShaderDataType dataType) : Location(location), DataType(dataType) {}
    };

    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoord;

        static std::vector<VertexShaderAttribute> GetAttribDescription()
        {
            return {{0, ShaderDataType::Float3}, {1, ShaderDataType::Float3}, {2, ShaderDataType::Float2}};
        }

        bool operator==(const Vertex& other) const
        {
            return Position == other.Position && Normal == other.Normal && TexCoord == other.TexCoord;
        }
    };

    class Shader : public RefCounted
    {
    public:
        static Ref<Shader> Create(std::string path);
        ~Shader() override;

        void Bind() const;
        void Reload();

        const std::string& GetPath() { return m_Path; }
        void SetPath(std::string path) { m_Path = std::move(path); }

    private:
        explicit Shader(std::string&& path);

        static std::vector<U32> Compile(const std::string& path, ShaderStage stage, const std::string& code);
        static ShaderSources PreProcess(const std::string& code);

        void CreateGraphicsPipeline(std::vector<U32>&& vertShaderBytecode, std::vector<U32>&& fragShaderBytecode);

    private:
        std::string m_Path;

        vk::Pipeline m_Pipeline = nullptr;
        vk::PipelineLayout m_Layout = nullptr;

        friend class Ref<Shader>;
    };

} // namespace Rose

template<>
struct std::hash<Rose::Vertex>
{
    size_t operator()(const Rose::Vertex& vertex) const noexcept
    {
        return ((hash<glm::vec3>()(vertex.Position) ^ (hash<glm::vec3>()(vertex.Normal) << 1)) >> 1) ^
               (hash<glm::vec2>()(vertex.TexCoord) << 1);
    }
};
