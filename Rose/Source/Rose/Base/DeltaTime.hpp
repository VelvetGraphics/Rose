#pragma once

#include <GLFW/glfw3.h>

namespace Rose {
    class DeltaTime
    {
    public:
        float Time;

    public:
        explicit DeltaTime(float time) : Time(time) {}

        void GetNextTime(float& lastTime)
        {
            float current = GetCurrentTime();
            Time = current - lastTime;
            lastTime = current;
        }

        static float GetCurrentTime() { return static_cast<float>(glfwGetTime()); }
    };
} // namespace Rose
