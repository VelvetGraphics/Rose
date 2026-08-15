#pragma once

#include "Rose/Base/InputCodes.hpp"

#include <GLFW/glfw3.h>

#include "Rose/Main/Main.hpp"

namespace Rose {
    class Input
    {
    public:
        static bool IsKeyPressed(Key::Key key)
        {
            return glfwGetKey(Main::GetWindow()->GetNativeWindow(), key) != GLFW_RELEASE;
        }

        static bool IsMouseButtonPressed(Mouse::Mouse button)
        {
            return glfwGetMouseButton(Main::GetWindow()->GetNativeWindow(), button) != GLFW_RELEASE;
        }
    };
} // namespace Rose
