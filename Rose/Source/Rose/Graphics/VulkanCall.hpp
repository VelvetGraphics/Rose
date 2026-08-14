#pragma once

#include "Rose/Graphics/VulkanInclude.hpp"

namespace Rose {
    class VulkanCall
    {
    public:
        static std::pair<vk::Buffer, vk::DeviceMemory> CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                                                    vk::MemoryPropertyFlags memProps);

        static void CopyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size);

        static std::pair<vk::Image, vk::DeviceMemory> CreateImage(vk::Extent3D extent, vk::Format format,
                                                                  vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                                                                  vk::MemoryPropertyFlags memProps);

        static vk::ImageView CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspect);

        static U32 FindMemoryTypeIndex(U32 typeFilter, vk::MemoryPropertyFlags flags);
    };
} // namespace Rose
