#include "Main.hpp"

#include "Rose/Base/DeltaTime.hpp"
#include "Rose/Base/EventTypes.hpp"
#include "Rose/Base/Window.hpp"
#include "Rose/Core/Core.hpp"
#include "Rose/Renderer/Renderer.hpp"

int Rose::Main::Run(std::vector<std::string>&& args)
{
    try
    {
        ASSERT(RoseInit(args), "Failed to initialize Rose");
        ASSERT(AppInitFn(std::move(args)), "Failed to initialize App");

        MainLoop();

        AppShutdownFn();
        RoseShutdown();
    }
    catch (const std::exception& e)
    {
        std::cout << "Caught exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

bool Rose::Main::RoseInit(std::vector<std::string>& args)
{
    s_Instance = this;

    Logger::Init();

    ASSERT(WindowContext::Create(), "Failed to create window context");
    auto info = Window::WindowInfoFn();

    m_Window = Ref<Window>::Create(info);
    m_Window->SetEventCallback(
            [ObjectPtr = &m_EventBus]<typename T0>(T0&& PH1) { ObjectPtr->Queue(std::forward<T0>(PH1)); });

    ASSERT(Renderer::Init(m_Window->GetNativeWindow()), "Failed to initialize renderer");

    m_EventBus.Observe<WindowClosedEvent>(std::bind(Rose::Main::OnWindowClose, std::placeholders::_1));
    m_LayerStack.SetEventBus(&m_EventBus);

    return true;
}

void Rose::Main::RoseShutdown()
{
    s_Running = true;

    Renderer::Shutdown();

    m_Window->Close();
    WindowContext::Destroy();

    Logger::Shutdown();

    s_Instance = nullptr;
}

void Rose::Main::MainLoop()
{
    float lastTime = DeltaTime::GetCurrentTime();
    DeltaTime dt(0.0f);
    dt.GetNextTime(lastTime);

    while (s_Running)
    {

        while (CanTick())
            m_LayerStack.Tick(s_TickTime);

        m_LayerStack.Update(dt.Time);

        if (!m_Window->IsMinimized())
        {
            if (Renderer::BeginFrame())
            {
                m_LayerStack.Render();
                Renderer::EndFrame();
            }
        }

        Window::PollEvents();
        dt.GetNextTime(lastTime);

        m_EventBus.Dispatch();
    }
}

bool Rose::Main::CanTick()
{
    if (m_TimeSum - s_TickTime >= 0)
    {
        m_TimeSum -= s_TickTime;
        return true;
    }

    return false;
}

int main(int argc, char** argv)
{
    std::vector<std::string> args(argc);
    for (int i = 0; i < argc; i++)
        args[i] = argv[i];

    Rose::Main main;
    return main.Run(std::move(args));
}

#if defined(ROSE_PLATFORM_WINDOWS) && defined(ROSE_DIST)
#include <windows.h>

#include <shellapi.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    std::vector<std::string> args(argc);
    std::wstring placeholder;
    for (int i = 0; i < argc; i++)
    {
        placeholder = argv[i];
        char* buffer = new char[wcstombs(nullptr, placeholder.c_str(), 0) + 1];
        args[i] = buffer;
        delete[] buffer;
    }

    LocalFree(argv);

    Rose::Main main;
    return main.Run(std::move(args));
}
#endif
