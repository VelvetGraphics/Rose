#include "Texture.hpp"

#include "Platform/Vulkan/Graphics/VulkanTexture.hpp"
#include "Rose/Core/Core.hpp"
#include "Rose/Graphics/GraphicsAPI.hpp"

namespace Rose {

    Ref<Texture> Texture::Create(std::string path, TextureSamplerInfo samplerInfo)
    {
        switch (GraphicsAPI::GetAPI())
        {
            case GraphicsAPI::Vulkan:
                return Ref<VulkanTexture>::Create(std::move(path), samplerInfo);
        }

        ASSERT(false, "Invalid graphics API");
        return {};
    }
} // namespace Rose
