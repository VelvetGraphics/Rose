#include "Shader.hpp"

#include "GraphicsAPI.hpp"
#include "Platform/Vulkan/Graphics/VulkanShader.hpp"
#include "Rose/Core/Core.hpp"

namespace Rose {
    Ref<Shader> Shader::Create(std::string path)
    {
        switch (GraphicsAPI::GetAPI())
        {
            case GraphicsAPI::Vulkan:
                return Ref<VulkanShader>::Create(std::move(path));
        }

        ASSERT(false, "Invalid graphics API");
        return {};
    }
} // namespace Rose
