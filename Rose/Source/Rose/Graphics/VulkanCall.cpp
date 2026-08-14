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

    void VulkanCall::CopyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size) {}

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
} // namespace Rose
