#include "ImGuiAPI.hpp"

#include "Platform/Vulkan/ImGui/VulkanImGuiAPI.hpp"
#include "Rose/Base/Window.hpp"
#include "Rose/Graphics/GraphicsAPI.hpp"

namespace Rose {
    void ImGuiAPI::Init(Ref<Window> window)
    {
        switch (GraphicsAPI::GetAPI())
        {
            case GraphicsAPI::Vulkan:
                return VulkanImGuiAPI::Init(window->GetNativeWindow());
        }
    }

    void ImGuiAPI::Shutdown()
    {
        switch (GraphicsAPI::GetAPI())
        {
            case GraphicsAPI::Vulkan:
                return VulkanImGuiAPI::Shutdown();
        }
    }

    void ImGuiAPI::BeginFrame()
    {
        switch (GraphicsAPI::GetAPI())
        case GraphicsAPI::Vulkan:
            return VulkanImGuiAPI::BeginFrame();
    }

    void ImGuiAPI::EndFrame()
    {
        switch (GraphicsAPI::GetAPI())
        case GraphicsAPI::Vulkan:
            return VulkanImGuiAPI::EndFrame();
    }
} // namespace Rose
