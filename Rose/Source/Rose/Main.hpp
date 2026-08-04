#pragma once

int main(int argc, char** argv);

namespace Rose {
    class Main
    {
    public:
        using AppInitFunc = bool (*)(std::vector<std::string>&& args);
        using AppShutdownFunc = void (*)();

        static AppInitFunc AppInitFn;
        static AppShutdownFunc AppShutdownFn;

    private:
        static int Run(std::vector<std::string>&& args);

        static bool RoseInit(std::vector<std::string>& args);
        static void RoseShutdown();

        friend int ::main(int argc, char** argv);
    };
} // namespace Rose
