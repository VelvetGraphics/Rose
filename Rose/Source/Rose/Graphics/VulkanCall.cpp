#include "VulkanCall.hpp"

#include "Rose/Core/Core.hpp"
#include "Rose/Graphics/GraphicsAPI.hpp"

namespace Rose {
    std::pair<vk::Buffer, vk::DeviceMemory> VulkanCall::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                                                     vk::MemoryPropertyFlags memProps)
    {
        vk::BufferCreateInfo bufferInfo = {};
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        auto bufferResult = GraphicsAPI::Device().createBuffer(bufferInfo);
        ASSERT(bufferResult.result == vk::Result::eSuccess, "Failed to create buffer");
        vk::Buffer buffer = bufferResult.value;

        vk::MemoryRequirements memRequirements = GraphicsAPI::Device().getBufferMemoryRequirements(buffer);
        vk::MemoryAllocateInfo allocInfo = {};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryTypeIndex(memRequirements.memoryTypeBits, memProps);

        auto allocResult = GraphicsAPI::Device().allocateMemory(allocInfo);
        ASSERT(allocResult.result == vk::Result::eSuccess, "Failed to allocate memory for buffer");

        vk::DeviceMemory bufferMemory = allocResult.value;
        ASSERT(GraphicsAPI::Device().bindBufferMemory(buffer, bufferMemory, 0) == vk::Result::eSuccess,
               "Failed to bind buffer memory");

        return {buffer, bufferMemory};
    }

    void VulkanCall::CopyBuffer(vk::CommandBuffer cmdBuffer, vk::Buffer src, vk::DeviceSize srcOffset, vk::Buffer dst,
                                vk::DeviceSize dstOffset, vk::DeviceSize size)
    {
        vk::BufferCopy region = {};
        region.size = size;
        region.srcOffset = srcOffset;
        region.dstOffset = dstOffset;

        cmdBuffer.copyBuffer(src, dst, region);
    }

    std::pair<vk::Image, vk::DeviceMemory> VulkanCall::CreateImage(vk::Extent3D extent, vk::Format format,
                                                                   vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                                                                   vk::MemoryPropertyFlags memProps)
    {
        vk::ImageCreateInfo imageInfo = {};
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.extent = extent;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.usage = usage;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.arrayLayers = 1;
        imageInfo.mipLevels = 1;
        imageInfo.samples = vk::SampleCountFlagBits::e1;

        auto imageResult = GraphicsAPI::Device().createImage(imageInfo);
        ASSERT(imageResult.result == vk::Result::eSuccess, "Failed to create image");
        vk::Image image = imageResult.value;

        vk::MemoryRequirements memRequirements = GraphicsAPI::Device().getImageMemoryRequirements(image);
        vk::MemoryAllocateInfo allocInfo = {};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryTypeIndex(memRequirements.memoryTypeBits, memProps);

        auto imageMemoryResult = GraphicsAPI::Device().allocateMemory(allocInfo);
        ASSERT(imageMemoryResult.result == vk::Result::eSuccess, "Failed to allocate image memory");
        vk::DeviceMemory imageMemory = imageMemoryResult.value;

        ASSERT(GraphicsAPI::Device().bindImageMemory(image, imageMemory, 0) == vk::Result::eSuccess,
               "Failed to bind image memory");
        return {image, imageMemory};
    }

    void VulkanCall::CopyBufferToImage(vk::CommandBuffer cmdBuffer, vk::Buffer buffer, vk::Image image,
                                       vk::Extent3D extent, vk::ImageAspectFlags aspect)
    {
        vk::BufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = aspect;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageSubresource.mipLevel = 0;
        region.imageOffset = vk::Offset3D{0, 0, 0};
        region.imageExtent = extent;

        cmdBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
    }

    vk::ImageView VulkanCall::CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspect)
    {
        vk::ImageViewCreateInfo viewInfo = {};
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.image = image;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspect;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseMipLevel = 0;

        auto imageViewResult = GraphicsAPI::Device().createImageView(viewInfo);
        ASSERT(imageViewResult.result == vk::Result::eSuccess, "Failed to create image view");
        vk::ImageView imageView = imageViewResult.value;

        return imageView;
    }

    U32 VulkanCall::FindMemoryTypeIndex(U32 typeFilter, vk::MemoryPropertyFlags flags)
    {
        vk::PhysicalDeviceMemoryProperties properties = GraphicsAPI::PhysicalDevice().getMemoryProperties();

        for (U32 i = 0; i < properties.memoryTypeCount; i++)
        {
            if (typeFilter & (1 << i) && (properties.memoryTypes[i].propertyFlags & flags) == flags)
                return i;
        }

        ASSERT(false, "No suitable memory type;");
    }

    void VulkanCall::TransitionImageLayout(vk::CommandBuffer cmdBuffer, vk::Image image, vk::ImageLayout oldLayout,
                                           vk::ImageLayout newLayout, vk::ImageAspectFlags aspect)
    {
        vk::ImageMemoryBarrier barrier = {};
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = aspect;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        vk::PipelineStageFlags sourceStage;
        vk::PipelineStageFlags destinationStage;

        if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
        {
            barrier.srcAccessMask = {};
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eTransfer;
        }
        else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
                 newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
        {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            sourceStage = vk::PipelineStageFlagBits::eTransfer;
            destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
        }
        else
        {
            ASSERT(false, "Unsupported image layout transition");
        }

        cmdBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);
    }
} // namespace Rose
