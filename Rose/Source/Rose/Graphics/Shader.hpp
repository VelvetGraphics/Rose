#pragma once

#include "Rose/Graphics/VulkanInclude.hpp"

#include <spirv_cross.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_CXX20
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include "Rose/Core/Core.hpp"
#include "Rose/Graphics/GraphicsAPI.hpp"

namespace Rose {
    using ShaderResourceKey = std::pair<U32, U32>;

    struct ShaderResource
    {
        std::string Name;
        vk::DescriptorType Type;

        U32 Set = U32Limit;
        U32 Binding = U32Limit;

        vk::DeviceSize Size;

        vk::ShaderStageFlags Stages;

        ShaderResourceKey Key() const { return {Set, Binding}; }
    };

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
        U32 Offset = 0;

        ShaderDataType DataType = ShaderDataType::None;

        VertexShaderAttribute(U32 location, U32 offset, ShaderDataType dataType) :
            Location(location), DataType(dataType)
        {
        }
    };

    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoord;

        static std::vector<VertexShaderAttribute> GetAttribDescription()
        {
            return {{0, offsetof(Vertex, Position), ShaderDataType::Float3},
                    {1, offsetof(Vertex, Normal), ShaderDataType::Float3},
                    {2, offsetof(Vertex, TexCoord), ShaderDataType::Float2}};
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

        template<typename T>
        void SetUBO(ShaderResourceKey key, T* ubo);

        const std::string& GetPath() { return m_Path; }
        void SetPath(std::string path) { m_Path = std::move(path); }

    private:
        explicit Shader(std::string&& path);

        static std::vector<U32> Compile(const std::string& path, ShaderStage stage, const std::string& code);
        static ShaderSources PreProcess(const std::string& code);

        void DestroyResources();

        void CreateDescriptors();
        void CreateUniformObjects();
        void CreateGraphicsPipeline(std::vector<U32>&& vertShaderBytecode, std::vector<U32>&& fragShaderBytecode);

        void BindDescriptorSet(U32 set) const;

    private:
        std::string m_Path;
        bool m_Loaded = false;

        vk::Pipeline m_Pipeline;
        vk::PipelineLayout m_Layout;

        std::map<ShaderResourceKey, ShaderResource> m_Resources;
        vk::DescriptorPool m_DescriptorPool;
        std::vector<vk::DescriptorSetLayout> m_DescriptorSetLayouts;
        std::vector<std::vector<vk::DescriptorSet>> m_DescriptorSets;

        std::map<ShaderResourceKey, std::vector<vk::Buffer>> m_UniformBuffers;
        std::map<ShaderResourceKey, std::vector<vk::DeviceMemory>> m_UniformBuffersMemory;
        std::map<ShaderResourceKey, std::vector<void*>> m_UniformBuffersMapped;

        friend class Ref<Shader>;
    };

    template<typename T>
    void Shader::SetUBO(ShaderResourceKey key, T* ubo)
    {
        ASSERT(ubo != nullptr, "UBO can NOT be nullptr");

        auto resourceIt = m_Resources.find(key);
        ASSERT(resourceIt != m_Resources.end(), "Shader resource does not exist");

        const ShaderResource& resource = resourceIt->second;
        ASSERT(resource.Type == vk::DescriptorType::eUniformBuffer, "Shader resource is not a uniform buffer");
        ASSERT(sizeof(T) == resource.Size, "Size does not match resource size");

        auto mappedIt = m_UniformBuffersMapped.find(key);
        ASSERT(mappedIt != m_UniformBuffersMapped.end(), "Uniform buffer has no mapped memory");

        const U32 frame = GraphicsAPI::FrameIndex();
        ASSERT(frame < mappedIt->second.size(), "Invalid frame index");

        memmove(mappedIt->second[frame], ubo, resource.Size);
    }
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
