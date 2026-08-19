#include "ViewportImage.hpp"

#include "Platform/Vulkan/Graphics/VulkanViewportImage.hpp"
#include "Rose/Core/Core.hpp"
#include "Rose/Graphics/GraphicsAPI.hpp"

namespace Rose {
    Ref<ViewportImage> ViewportImage::Create()
    {
        switch (GraphicsAPI::GetAPI())
        {
            case GraphicsAPI::Vulkan:
                return Ref<VulkanViewportImage>::Create();
        }

        ASSERT(false, "Invalid graphics API");
        return {};
    }
} // namespace Rose
