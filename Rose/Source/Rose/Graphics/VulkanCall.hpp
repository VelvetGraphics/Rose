#pragma once

#include "Rose/Graphics/VulkanInclude.hpp"

namespace Rose {
    class VulkanCall
    {
    public:
        static std::pair<vk::Buffer, vk::DeviceMemory> CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                                                    vk::MemoryPropertyFlags memProps);

        static void CopyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size);

        static U32 FindMemoryTypeIndex(U32 typeFilter, vk::MemoryPropertyFlags flags);
    };
} // namespace Rose
