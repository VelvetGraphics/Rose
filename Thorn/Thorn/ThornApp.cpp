#include "ThornApp.hpp"
#include "Thorn/SceneHierarchyPanelLayer.hpp"
#include "Thorn/ViewportLayer.hpp"

#include <Rose/Rose.hpp>

namespace Rose {
    bool ThornApp::Init(std::vector<std::string>&& args)
    {
        s_Viewport = ViewportImage::Create();
        Renderer::SetViewport(*s_Viewport);

        s_ViewportLayer = new ViewportLayer;
        Main::Instance()->PushLayer(s_ViewportLayer);

        s_SceneHierarchyPanel = new SceneHierarchyPanelLayer;
        Main::Instance()->PushOverlay(s_SceneHierarchyPanel);

        return true;
    }

    namespace {
        WindowInfo ThornWindowInfo() { return {}; }
    } // namespace

    Main::AppInitFunc Main::AppInitFn = ThornApp::Init;
    Main::AppShutdownFunc Main::AppShutdownFn = ThornApp::ShutDown;
    bool Main::ViewportRendering = true;
    Window::WindowInfoFunc Window::WindowInfoFn = ThornWindowInfo;
} // namespace Rose
