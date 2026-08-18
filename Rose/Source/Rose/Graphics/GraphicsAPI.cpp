#include "GraphicsAPI.hpp"

#include "Platform/Vulkan/Graphics/VulkanAPI.hpp"

namespace Rose {
    GraphicsAPI* GraphicsAPI::Create(GLFWwindow* window)
    {
        switch (s_API)
        {
            case Vulkan:
                return new VulkanAPI(window);
        }

        ASSERT(false, "Invalid graphics API");
        return nullptr;
    }
} // namespace Rose
