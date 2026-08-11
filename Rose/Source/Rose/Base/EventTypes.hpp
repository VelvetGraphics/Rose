#pragma once

#include "Rose/Base/Event.hpp"
#include "Rose/Base/InputCodes.hpp"

namespace Rose {
    class Window;

    class AppTickedEvent final : public Event
    {
    public:
        EVENT_CLASS_TYPE(EventType::AppTicked)
        EVENT_CLASS_CATEGORY(EventCategory::App)
    };

    class AppUpdatedEvent final : public Event
    {
    public:
        EVENT_CLASS_TYPE(EventType::AppTicked)
        EVENT_CLASS_CATEGORY(EventCategory::App)
    };

    class AppRenderedEvent final : public Event
    {
    public:
        EVENT_CLASS_TYPE(EventType::AppRendered)
        EVENT_CLASS_CATEGORY(EventCategory::App)
    };

    class WindowEvent : public Event
    {
    public:
        ~WindowEvent() override = default;

        EVENT_CLASS_CATEGORY(EventCategory::App)

        Window* GetWindow() const { return m_Window; }

    protected:
        explicit WindowEvent(Window* window) : m_Window(window) {}

    private:
        Window* m_Window;
    };

    class WindowClosedEvent final : public WindowEvent
    {
    public:
        explicit WindowClosedEvent(Window* window) : WindowEvent(window) {}

        EVENT_CLASS_TYPE(EventType::WindowClosed)
    };

    class WindowResizedEvent final : public WindowEvent
    {
    public:
        WindowResizedEvent(Window* window, uint32_t width, uint32_t height) :
            WindowEvent(window), m_Width(width), m_Height(height)
        {
        }

        EVENT_CLASS_TYPE(EventType::WindowResized)

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }

    private:
        uint32_t m_Width = 0, m_Height = 0;
    };

    class WindowFocusedEvent final : public WindowEvent
    {
    public:
        explicit WindowFocusedEvent(Window* window) : WindowEvent(window) {}

        EVENT_CLASS_TYPE(EventType::WindowFocused)
    };

    class WindowLostFocusEvent final : public WindowEvent
    {
    public:
        explicit WindowLostFocusEvent(Window* window) : WindowEvent(window) {}

        EVENT_CLASS_TYPE(EventType::WindowLostFocus)
    };

    class WindowMinimizedEvent final : public WindowEvent
    {
    public:
        explicit WindowMinimizedEvent(Window* window) : WindowEvent(window) {}

        EVENT_CLASS_TYPE(EventType::WindowMinimized)
    };

    class WindowUnMinimizedEvent final : public WindowEvent
    {
    public:
        explicit WindowUnMinimizedEvent(Window* window) : WindowEvent(window) {}

        EVENT_CLASS_TYPE(EventType::WindowUnMinimized)
    };

    class WindowMovedEvent final : public WindowEvent
    {
    public:
        WindowMovedEvent(Window* window, int xPos, int yPos) : WindowEvent(window), m_XPos(xPos), m_YPos(yPos) {}

        EVENT_CLASS_TYPE(EventType::WindowMoved)

    private:
        int m_XPos, m_YPos;
    };

    class KeyEvent : public Event
    {
    public:
        ~KeyEvent() override = default;

        EVENT_CLASS_CATEGORY(EventCategory::Input | EventCategory::Keyboard)

        Key::Key GetKeyCode() const { return m_KeyCode; }

    protected:
        explicit KeyEvent(Key::Key keyCode) : m_KeyCode(keyCode) {}

    private:
        Key::Key m_KeyCode;
    };

    class KeyPressedEvent final : public KeyEvent
    {
    public:
        KeyPressedEvent(Key::Key keyCode, int repeatCount) : KeyEvent(keyCode), m_RepeatCount(repeatCount) {}

        EVENT_CLASS_TYPE(EventType::KeyPressed)

    private:
        int m_RepeatCount;
    };

    class KeyReleasedEvent final : public KeyEvent
    {
    public:
        explicit KeyReleasedEvent(Key::Key keyCode) : KeyEvent(keyCode) {}

        EVENT_CLASS_TYPE(EventType::KeyReleased)
    };

    class MouseButtonEvent : public Event
    {
    public:
        ~MouseButtonEvent() override = default;

        EVENT_CLASS_CATEGORY(EventCategory::Input | EventCategory::Mouse)

        Mouse::Mouse GetMouseButton() const { return m_Button; }

    protected:
        explicit MouseButtonEvent(Mouse::Mouse button) : m_Button(button) {}

    private:
        Mouse::Mouse m_Button;
    };

    class MouseButtonPressedEvent final : public MouseButtonEvent
    {
    public:
        explicit MouseButtonPressedEvent(Mouse::Mouse button) : MouseButtonEvent(button) {}

        EVENT_CLASS_TYPE(EventType::MouseButtonPressed);
    };

    class MouseButtonReleasedEvent final : public MouseButtonEvent
    {
    public:
        explicit MouseButtonReleasedEvent(Mouse::Mouse button) : MouseButtonEvent(button) {}

        EVENT_CLASS_TYPE(EventType::MouseButtonReleased);
    };

    class MouseMovedEvent final : public Event
    {
    public:
        MouseMovedEvent(float xPos, float yPos) : m_XPos(xPos), m_YPos(yPos) {}

        EVENT_CLASS_TYPE(EventType::MouseScrolled)
        EVENT_CLASS_CATEGORY(EventCategory::Input | EventCategory::Mouse);

        float GetXPos() const { return m_XPos; }
        float GetYPos() const { return m_YPos; }

    private:
        float m_XPos, m_YPos;
    };

    class MouseScrolledEvent final : public Event
    {
    public:
        MouseScrolledEvent(float xOffset, float yOffset) : m_XOffset(xOffset), m_YOffset(yOffset) {}

        EVENT_CLASS_TYPE(EventType::MouseScrolled)
        EVENT_CLASS_CATEGORY(EventCategory::Input | EventCategory::Mouse)

        float GetXOffset() const { return m_XOffset; }
        float GetYOffset() const { return m_YOffset; }

    private:
        float m_XOffset, m_YOffset;
    };
} // namespace Rose
