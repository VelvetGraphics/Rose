#include "Mesh.hpp"

#include "Platform/Vulkan/Graphics/VulkanMesh.hpp"
#include "Rose/Core/Core.hpp"
#include "Rose/Graphics/GraphicsAPI.hpp"

namespace Rose {

    Ref<Mesh> Mesh::Create(std::string path)
    {
        switch (GraphicsAPI::GetAPI())
        {
            case GraphicsAPI::Vulkan:
                return Ref<VulkanMesh>::Create(std::move(path));
        }

        ASSERT(false, "Invalid graphics API");
        return {};
    }
} // namespace Rose
