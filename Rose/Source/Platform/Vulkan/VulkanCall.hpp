#pragma once

#include "VulkanInclude.hpp"

namespace Rose {
    class VulkanCall
    {
    public:
        static std::pair<vk::Buffer, vk::DeviceMemory> CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                                                    vk::MemoryPropertyFlags memProps);

        static void CopyBuffer(vk::CommandBuffer cmdBuffer, vk::Buffer src, vk::DeviceSize srcOffset, vk::Buffer dst,
                               vk::DeviceSize dstOffset, vk::DeviceSize size);

        static std::pair<vk::Image, vk::DeviceMemory> CreateImage(vk::Extent3D extent, vk::Format format,
                                                                  vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                                                                  vk::MemoryPropertyFlags memProps, U32 mipLevels);

        static void CopyBufferToImage(vk::CommandBuffer cmdBuffer, vk::Buffer buffer, vk::Image image,
                                      vk::Extent3D extent, vk::ImageAspectFlags aspect);

        static vk::ImageView CreateImageView(vk::Image image, vk::Format format, U32 mipLevels,
                                             vk::ImageAspectFlags aspect);

        static U32 FindMemoryTypeIndex(U32 typeFilter, vk::MemoryPropertyFlags flags);

        static void TransitionImageLayout(vk::CommandBuffer cmdBuffer, vk::Image image, vk::ImageLayout oldLayout,
                                          vk::ImageLayout newLayout, U32 mipLevels, vk::ImageAspectFlags aspect);
    };
} // namespace Rose
