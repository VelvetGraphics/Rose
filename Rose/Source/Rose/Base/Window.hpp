#pragma once
#include "Rose/Base/Event.hpp"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

namespace Rose {
    struct WindowInfo
    {
        std::string Title = "Rose";

        int Width = 1280, Height = 720;

        bool HasTitlebar = true;
        bool Resizable = true;
        bool Fullscreen = false;
    };

    class Window final : public RefCounted
    {
        using EventCallbackFunc = std::function<void(std::unique_ptr<Event>)>;

    public:
        using WindowInfoFunc = std::function<WindowInfo()>;
        static WindowInfoFunc WindowInfoFn;

    public:
        explicit Window(const WindowInfo& info);
        ~Window() override;

        void Close();
        bool IsOpen() const { return m_Open; }

        int GetWidth() const { return m_Data.Width; }
        void SetWidth(int width);

        int GetHeight() const { return m_Data.Height; }
        void SetHeight(int height);

        glm::ivec2 GetSize() const { return {m_Data.Width, m_Data.Height}; }
        void SetSize(const glm::ivec2& size);

        bool IsFocused() const { return m_Data.Focused; }
        bool IsMinimized() const { return m_Data.Minimized; }

        GLFWwindow* GetNativeWindow() const { return m_Window; }
        void SetEventCallback(const EventCallbackFunc& callback) { m_Data.EventCallbackFn = callback; }

        static void PollEvents() { glfwPollEvents(); }

    private:
        void SetEventCallbackFunctions() const;

    private:
        GLFWwindow* m_Window;
        bool m_Open = false;

        struct WindowData
        {
            WindowData() = default;
            WindowData(const WindowInfo& info, GLFWwindow* window) :
                Title(info.Title), Width(info.Width), Height(info.Height)
            {
                glfwGetWindowPos(window, &XPos, &YPos);
            }

            Window* Self = nullptr;
            EventCallbackFunc EventCallbackFn = nullptr;

            std::string Title;

            int Width = 0, Height = 0;
            int XPos = 0, YPos = 0;

            bool Open = true;
            bool Focused = true;
            bool Minimized = false;
        };

        WindowData m_Data;
    };

    class WindowContext final
    {
    public:
        static bool Create()
        {
            if (glfwInit())
            {
                s_Initialized = true;
                return true;
            }

            return false;
        }

        static void Destroy()
        {
            glfwTerminate();
            s_Initialized = false;
        }

        static bool IsInitialized() { return s_Initialized; }

    private:
        inline static bool s_Initialized = false;
    };
} // namespace Rose
