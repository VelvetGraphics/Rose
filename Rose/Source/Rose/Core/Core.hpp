#pragma once

#include "Rose/Core/Logger.hpp"

// TODO: No assertions in dist builds
#define ASSERT(expr, ...)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!expr)                                                                                                     \
        {                                                                                                              \
            Logger::Error("Assertion Failed with expression: {0}\n message: {1}\n\n file: {2},\n line: {3}", #expr,    \
                          __VA_ARGS__, __FILE__, __LINE__);                                                            \
            std::abort();                                                                                              \
        }                                                                                                              \
    } while (false)
