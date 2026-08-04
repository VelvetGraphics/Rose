#pragma once

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace Rose {
    class Logger
    {
    public:
        static void Init()
        {
            spdlog::set_pattern("%^[%T:%e] %n (%l) : %v%$");

            s_Logger = spdlog::stdout_color_mt("Rose");
            s_Logger->set_level(spdlog::level::trace);

            s_Initialized = true;
        }

        static void Shutdown()
        {
            s_Logger.reset();
            s_Initialized = false;
        }

#if !defined(ROSE_DIST)
        template<typename... Args>
        static void Trace(spdlog::format_string_t<Args...> format, Args&&... args)
        {
            s_Logger->trace(format, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Info(spdlog::format_string_t<Args...> format, Args&&... args)
        {
            s_Logger->info(format, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Warn(spdlog::format_string_t<Args...> format, Args&&... args)
        {
            s_Logger->warn(format, std::forward<Args>(args)...);
        }
#else
        template<typename... Args>
        static void Trace(spdlog::format_string_t<Args...> format, Args&&... args)
        {
        }

        template<typename... Args>
        static void Info(spdlog::format_string_t<Args...> format, Args&&... args)
        {
        }

        template<typename... Args>
        static void Warn(spdlog::format_string_t<Args...> format, Args&&... args)
        {
        }
#endif

        template<typename... Args>
        static void Error(spdlog::format_string_t<Args...> format, Args&&... args)
        {
            s_Logger->error(format, std::forward<Args>(args)...);
        }

    private:
        inline static std::shared_ptr<spdlog::logger> s_Logger;
        inline static bool s_Initialized = false;
    };
} // namespace Rose
