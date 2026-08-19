#pragma once
#include <Rose/Rose.hpp>

namespace Rose {
    class ViewportLayer;
    class SceneHierarchyPanelLayer;
    class ThornApp
    {
    public:
        static bool Init(std::vector<std::string>&& args);
        static void ShutDown() { s_Viewport = {}; }

        static Ref<ViewportImage>& GetViewport() { return s_Viewport; }
        static ViewportLayer* GetViewportLayer() { return s_ViewportLayer; }

        static SceneHierarchyPanelLayer* GetSceneHierarchyPanel() { return s_SceneHierarchyPanel; }

    private:
        inline static Ref<ViewportImage> s_Viewport;
        inline static ViewportLayer* s_ViewportLayer;
        inline static SceneHierarchyPanelLayer* s_SceneHierarchyPanel;
    };
} // namespace Rose
