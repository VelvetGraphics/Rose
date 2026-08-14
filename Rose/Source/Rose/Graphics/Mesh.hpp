#pragma once

#include "Rose/Graphics/Shader.hpp"
#include "Rose/Graphics/VulkanInclude.hpp"

namespace Rose {
    class Mesh : public RefCounted
    {
    public:
        static Ref<Mesh> Create(std::string path);
        ~Mesh() override;

        void Reload();
        void Bind() const;
        void Draw() const;

        const std::string& GetPath() { return m_Path; }
        void SetPath(std::string path) { m_Path = std::move(path); }

    private:
        explicit Mesh(std::string&& path);

        std::pair<std::vector<Vertex>, std::vector<U32>> LoadObj();

        static std::pair<vk::Buffer, vk::DeviceMemory> CreateBuffer(void* data, vk::DeviceSize size,
                                                                    vk::BufferUsageFlagBits usage);

    private:
        std::string m_Path;

        vk::Buffer m_VertexBuffer;
        vk::DeviceMemory m_VertexBufferMemory;
        U32 m_VertexCount;

        vk::Buffer m_IndexBuffer;
        vk::DeviceMemory m_IndexBufferMemory;
        U32 m_IndexCount;

        friend Ref<Mesh>;
    };
} // namespace Rose
