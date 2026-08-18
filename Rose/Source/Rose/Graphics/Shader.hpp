#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_CXX20
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace Rose {
    class Texture;
    using ShaderResourceKey = std::pair<U32, U32>;

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
        U32 Location = U32Limit;
        U32 Offset = U32Limit;

        ShaderDataType DataType = ShaderDataType::None;

        VertexShaderAttribute(U32 location, U32 offset, ShaderDataType dataType) :
            Location(location), Offset(offset), DataType(dataType)
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
        ~Shader() override = default;

        static Ref<Shader> Create(std::string path);

        virtual void Bind() const = 0;
        virtual void Reload() = 0;

        virtual void SetUBO(ShaderResourceKey key, const void* ubo, U64 size) = 0;
        virtual void SetTexture(ShaderResourceKey key, const Ref<Texture>& texture) = 0;

        const std::string& GetPath() { return m_Path; }
        void SetPath(std::string path) { m_Path = std::move(path); }

    protected:
        Shader(std::string&& path) : m_Path(std::move(path)) {}

    protected:
        std::string m_Path;
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
