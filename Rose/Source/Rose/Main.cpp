#include "Main.hpp"

#include "Rose/Core/Core.hpp"

int Rose::Main::Run(std::vector<std::string>&& args)
{
    try
    {
        ASSERT(RoseInit(args), "Failed to initialize Rose");
        ASSERT(AppInitFn(std::move(args)), "Failed to initialize App");

        // TODO: App loop

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
    Logger::Init();

    return true;
}

void Rose::Main::RoseShutdown() { Logger::Shutdown(); }

int main(int argc, char** argv)
{
    std::vector<std::string> args(argc);
    for (int i = 0; i < argc; i++)
        args[i] = argv[i];

    return Rose::Main::Run(std::move(args));
}

#if defined(ROSE_PLATFORM_WINDOWS) && defined(ROSE_DIST)
#include <windows.h>

#include <shellapi.h>

int main(int argc, char** argv)
{
    std::vector<std::string> args(argc);
    for (int i = 0; i < argc; i++)
        args[i] = argv[i];

    return Rose::Main::Run(std::move(args));
}

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

    Rose::Main::Run(std::move(args));
}
#endif
