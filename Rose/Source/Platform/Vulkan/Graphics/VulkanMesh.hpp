#pragma once

#include "Platform/Vulkan/Graphics/VulkanShader.hpp"
#include "Platform/Vulkan/VulkanInclude.hpp"
#include "Rose/Graphics/Mesh.hpp"

namespace Rose {
    class VulkanMesh : public Mesh
    {
    public:
        explicit VulkanMesh(std::string&& path);
        ~VulkanMesh() override;

        void Reload() override;
        void Bind() const override;
        void Draw() const override;

    private:
        std::pair<std::vector<Vertex>, std::vector<U32>> LoadObj() const;

        static std::pair<vk::Buffer, vk::DeviceMemory> CreateBuffer(const void* data, vk::DeviceSize size,
                                                                    vk::BufferUsageFlagBits usage);

    private:
        vk::Buffer m_VertexBuffer;
        vk::DeviceMemory m_VertexBufferMemory;
        U32 m_VertexCount = 0;

        vk::Buffer m_IndexBuffer;
        vk::DeviceMemory m_IndexBufferMemory;
        U32 m_IndexCount = 0;
    };
} // namespace Rose
