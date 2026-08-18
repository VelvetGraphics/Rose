#include "VulkanMesh.hpp"

#include "Platform/Vulkan/Graphics/VulkanAPI.hpp"
#include "Platform/Vulkan/VulkanCall.hpp"
#include "Rose/Core/Core.hpp"

#include <tiny_obj_loader/tiny_obj_loader.h>

#include "Platform/Vulkan/Graphics/VulkanShader.hpp"

namespace Rose {
    VulkanMesh::~VulkanMesh()
    {
        VulkanAPI::Device().freeMemory(m_IndexBufferMemory);
        VulkanAPI::Device().destroyBuffer(m_IndexBuffer);

        VulkanAPI::Device().freeMemory(m_VertexBufferMemory);
        VulkanAPI::Device().destroyBuffer(m_VertexBuffer);
    }

    void VulkanMesh::Reload()
    {
        auto [vertices, indices] = LoadObj();
        m_VertexCount = vertices.size();
        m_IndexCount = indices.size();

        std::tie(m_VertexBuffer, m_VertexBufferMemory) =
                CreateBuffer(vertices.data(), vertices.size() * sizeof(Vertex), vk::BufferUsageFlagBits::eVertexBuffer);

        std::tie(m_IndexBuffer, m_IndexBufferMemory) =
                CreateBuffer(indices.data(), indices.size() * sizeof(U32), vk::BufferUsageFlagBits::eIndexBuffer);
    }

    void VulkanMesh::Bind() const
    {
        VulkanAPI::Submit([this](vk::CommandBuffer cmdBuffer) {
            cmdBuffer.bindVertexBuffers(0, m_VertexBuffer, {0});
            cmdBuffer.bindIndexBuffer(m_IndexBuffer, 0, vk::IndexType::eUint32);
        });
    }

    void VulkanMesh::Draw() const
    {
        VulkanAPI::Submit([this](vk::CommandBuffer cmdBuffer) { cmdBuffer.drawIndexed(m_IndexCount, 1, 0, 0, 0); });
    }

    VulkanMesh::VulkanMesh(std::string&& path) : Mesh(std::move(path)) { VulkanMesh::Reload(); }

    std::pair<std::vector<Vertex>, std::vector<U32>> VulkanMesh::LoadObj() const
    {
        std::string baseDir = std::filesystem::path(m_Path).parent_path().string();

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string warn;
        std::string err;

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, m_Path.c_str(), baseDir.c_str());

        ASSERT(warn.empty(), warn);
        ASSERT(err.empty(), err);
        ASSERT(ret, "Failed to read file");

        std::vector<Vertex> vertices;
        std::vector<U32> indices;

        std::unordered_map<Vertex, U32> uniqueVertices;

        for (const auto& shape : shapes)
        {
            for (const auto& idx : shape.mesh.indices)
            {
                Vertex vertex = {};

                vertex.Position.x = attrib.vertices[3u * idx.vertex_index + 0];
                vertex.Position.y = attrib.vertices[3u * idx.vertex_index + 1];
                vertex.Position.z = attrib.vertices[3u * idx.vertex_index + 2];

                if (idx.normal_index >= 0)
                {
                    vertex.Normal.x = attrib.normals[3u * idx.normal_index + 0];
                    vertex.Normal.y = attrib.normals[3u * idx.normal_index + 1];
                    vertex.Normal.z = attrib.normals[3u * idx.normal_index + 2];
                }

                if (idx.texcoord_index >= 0)
                {
                    vertex.TexCoord.x = attrib.texcoords[2u * idx.texcoord_index + 0];
                    vertex.TexCoord.y = attrib.texcoords[2u * idx.texcoord_index + 1];
                }

                auto [it, inserted] = uniqueVertices.insert({vertex, static_cast<U32>(vertices.size())});

                if (inserted)
                    vertices.push_back(vertex);

                indices.push_back(it->second);
            }
        }

        return {vertices, indices};
    }

    std::pair<vk::Buffer, vk::DeviceMemory> VulkanMesh::CreateBuffer(const void* data, vk::DeviceSize size,
                                                                     vk::BufferUsageFlagBits usage)
    {
        auto [stagingBuffer, stagingBufferMemory] = VulkanCall::CreateBuffer(
                size, usage | vk::BufferUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        void* stagingData = VulkanAPI::Device().mapMemory(stagingBufferMemory, 0, size).value;
        memcpy(stagingData, data, size);

        auto [buffer, bufferMemory] = VulkanCall::CreateBuffer(size, usage | vk::BufferUsageFlagBits::eTransferDst,
                                                               vk::MemoryPropertyFlagBits::eDeviceLocal);

        VulkanAPI::SubmitSingleTime(
                [&](vk::CommandBuffer cmdBuffer) {
                    VulkanCall::CopyBuffer(cmdBuffer, stagingBuffer, 0, buffer, 0, size);
                },
                QueueType::Transfer);

        VulkanAPI::Device().freeMemory(stagingBufferMemory);
        VulkanAPI::Device().destroyBuffer(stagingBuffer);

        return {buffer, bufferMemory};
    }
} // namespace Rose
