#pragma once

namespace Rose {
    class Main
    {
    public:
        using AppInitFunc = bool (*)(std::vector<std::string>&& args);
        using AppShutdownFunc = void (*)();

        static AppInitFunc AppInitFn;
        static AppShutdownFunc AppShutdownFn;

    public:
        static int Run(std::vector<std::string>&& args);

    private:
        static bool RoseInit(std::vector<std::string>& args);
        static void RoseShutdown();
    };
} // namespace Rose
