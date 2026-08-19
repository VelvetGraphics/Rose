#pragma once
#include "Rose/Base/Window.hpp"
#include "Rose/Common/CommonInclude.hpp"

namespace Rose {
    class ImGuiAPI
    {
    public:
        ImGuiAPI() = delete;
        ~ImGuiAPI() = delete;

        static void Init(Ref<Window> window);
        static void Shutdown();

        static void BeginFrame();
        static void EndFrame();
    };
} // namespace Rose
