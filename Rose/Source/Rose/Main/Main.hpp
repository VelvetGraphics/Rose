#pragma once

#include "Rose/Base/EventTypes.hpp"
#include "Rose/Base/Layer.hpp"
#include "Rose/Base/Window.hpp"

namespace Rose {
    class Main final
    {
    public:
        using AppInitFunc = bool (*)(std::vector<std::string>&& args);
        using AppShutdownFunc = void (*)();

        static AppInitFunc AppInitFn;
        static AppShutdownFunc AppShutdownFn;

        static bool ViewportRendering;

    public:
        int Run(std::vector<std::string>&& args);
        static void Close() { s_Running = false; }
        static void OnWindowClose(WindowClosedEvent&) { Close(); }

        static Main* Instance() { return s_Instance; }
        static Ref<Window>& GetWindow() { return s_Instance->m_Window; }
        static EventBus& GetEventBus() { return s_Instance->m_EventBus; }

        void PushLayer(Layer* layer) { m_LayerStack.PushLayer(layer); }
        void PopLayer(Layer* layer) { m_LayerStack.PopLayer(layer); }

        void PushOverlay(Layer* overlay) { m_LayerStack.PushOverlay(overlay); }
        void PopOverlay(Layer* overlay) { m_LayerStack.PopOverlay(overlay); }

    private:
        bool RoseInit(std::vector<std::string>& args);
        void RoseShutdown();

        void MainLoop();

        bool CanTick();

    private:
        inline static Main* s_Instance;

        EventBus m_EventBus;
        Ref<Window> m_Window;

        LayerStack m_LayerStack;

        std::thread m_RenderThread;

        static constexpr float s_TickTime = 1.0f / 60.0f;
        float m_TimeSum = 0.0f;

        inline static bool s_Running = true;
    };
} // namespace Rose
