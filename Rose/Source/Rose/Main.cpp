#include "Main.hpp"

#include "Rose/Core/Core.hpp"

int main(int argc, char** argv)
{
    std::vector<std::string> args(argc);
    for (int i = 0; i < argc; i++)
        args[i] = argv[i];

    return Rose::Main::Run(std::move(args));
}

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
