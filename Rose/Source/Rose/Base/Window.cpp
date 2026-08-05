#include "Window.hpp"

#include "Rose/Base/EventTypes.hpp"

namespace Rose {
    Window::Window(const WindowInfo& info)
    {
        ASSERT(WindowContext::IsInitialized(), "Window context is NOT initialized");

        glfwWindowHint(GLFW_DECORATED, info.HasTitlebar);
        glfwWindowHint(GLFW_RESIZABLE, info.Resizable);

        m_Window = glfwCreateWindow(info.Width, info.Height, info.Title.c_str(),
                                    info.Fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

        m_Data = {info, m_Window};
        m_Data.Self = this;

        glfwSetWindowUserPointer(m_Window, &m_Data);

        SetEventCallbackFunctions();
    }

    Window::~Window()
    {
        if (m_Data.Open)
            Close();
    }

    void Window::Close()
    {
        glfwDestroyWindow(m_Window);
        m_Data.Open = false;
    }

    void Window::SetWidth(int width)
    {
        glfwSetWindowSize(m_Window, width, m_Data.Height);
        m_Data.Width = width;
    }

    void Window::SetHeight(int height)
    {
        glfwSetWindowSize(m_Window, m_Data.Width, height);
        m_Data.Height = height;
    }

    void Window::SetSize(const glm::ivec2& size)
    {
        glfwSetWindowSize(m_Window, size.x, size.y);
        m_Data.Width = size.x;
        m_Data.Height = size.y;
    }

    void Window::SetEventCallbackFunctions() const
    {
        glfwSetErrorCallback(
                [](int error, const char* description) { Logger::Error("GLFW ERROR ({0}), {1}", error, description); });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action)
            {
                case GLFW_PRESS:
                    data->EventCallbackFn(std::make_unique<KeyPressedEvent>(static_cast<Key::Key>(key), 0));
                    break;
                case GLFW_REPEAT:
                    data->EventCallbackFn(std::make_unique<KeyPressedEvent>(static_cast<Key::Key>(key), 1));
                    break;
                case GLFW_RELEASE:
                    data->EventCallbackFn(std::make_unique<KeyReleasedEvent>(static_cast<Key::Key>(key)));
                    break;
                default:;
            }
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
            auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action)
            {
                case GLFW_PRESS:
                    data->EventCallbackFn(std::make_unique<MouseButtonPressedEvent>(static_cast<Mouse::Mouse>(button)));
                    break;
                case GLFW_RELEASE:
                    data->EventCallbackFn(
                            std::make_unique<MouseButtonReleasedEvent>(static_cast<Mouse::Mouse>(button)));
                    break;
                default:;
            }
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
            auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            data->EventCallbackFn(
                    std::make_unique<MouseMovedEvent>(static_cast<float>(xPos), static_cast<float>(yPos)));
        });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
            auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            data->EventCallbackFn(
                    std::make_unique<MouseScrolledEvent>(static_cast<float>(xOffset), static_cast<float>(yOffset)));
        });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
            auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            data->EventCallbackFn(std::make_unique<WindowClosedEvent>(data->Self));
        });

        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
            auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            data->EventCallbackFn(std::make_unique<WindowResizedEvent>(data->Self, width, height));
        });

        glfwSetWindowFocusCallback(m_Window, [](GLFWwindow* window, int focused) {
            auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data->Focused = focused;

            if (focused)
                data->EventCallbackFn(std::make_unique<WindowFocusedEvent>(data->Self));
            else
                data->EventCallbackFn(std::make_unique<WindowLostFocusEvent>(data->Self));
        });

        glfwSetWindowPosCallback(m_Window, [](GLFWwindow* window, int xPos, int yPos) {
            auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            data->EventCallbackFn(std::make_unique<WindowMovedEvent>(data->Self, xPos, yPos));
        });
    }
} // namespace Rose
