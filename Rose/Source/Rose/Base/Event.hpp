#pragma once
#include "Rose/Core/Core.hpp"

namespace Rose {
    enum class EventType
    {
        None = 0,

        AppTicked,
        AppUpdated,
        AppRendered,

        WindowClosed,
        WindowResized,
        WindowFocused,
        WindowLostFocus,
        WindowMinimized,
        WindowUnMinimized,
        WindowMoved,

        KeyPressed,
        KeyReleased,

        MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        MouseScrolled
    };

    enum class EventCategory
    {
        None = 0,

        App = BIT(1),
        Input = BIT(2),
        Keyboard = BIT(3),
        Mouse = BIT(4),
    };

    constexpr EventCategory operator&(EventCategory a, EventCategory b)
    {
        return static_cast<EventCategory>(static_cast<int>(a) & static_cast<int>(b));
    }

    constexpr EventCategory operator|(EventCategory a, EventCategory b)
    {
        return static_cast<EventCategory>(static_cast<int>(a) | static_cast<int>(b));
    }

    class Event
    {
    public:
        virtual ~Event() = default;

    public:
        bool Handled = false;

    public:
        virtual EventType GetType() const { return EventType::None; }
        virtual EventCategory GetCategoryFlags() const { return EventCategory::None; }

#define EVENT_CLASS_TYPE(type)                                                                                         \
    static Rose::EventType Type() { return type; }                                                                     \
    Rose::EventType GetType() const override { return type; }

#define EVENT_CLASS_CATEGORY(category)                                                                                 \
    virtual Rose::EventCategory GetCategoryFlags() const override { return category; }
    };

    class EventBus final
    {
    public:
        EventBus() = default;
        ~EventBus() = default;

        EventBus(const EventBus&) = delete;
        EventBus& operator=(const EventBus&) = delete;

        EventBus(EventBus&&) noexcept = default;
        EventBus& operator=(EventBus&&) = default;

        template<typename T>
            requires std::derived_from<T, Event>
        using Callback = std::function<void(T&)>;

        template<typename T>
            requires std::derived_from<T, Event>
        void Observe(Callback<T> callback)
        {
            m_Observers[T::Type()].push_back([callback](Event& event) { callback(static_cast<T&>(event)); });
        }

        void Queue(std::unique_ptr<Event> event) { m_Queue.push_back(std::move(event)); }

        void Dispatch()
        {
            SwapQueues();

            for (std::unique_ptr<Event>& event : m_FrontQueue)
            {
                auto it = m_Observers.find(event->GetType());

                if (it == m_Observers.end())
                    continue;

                for (Callback<Event>& callback : it->second)
                    callback(*event);
            }

            m_FrontQueue.clear();
        }

        void ClearQueue() { m_Queue.clear(); }

    private:
        void SwapQueues() { std::swap(m_Queue, m_FrontQueue); }

    private:
        using EventCallback = std::function<void(Event&)>;

        std::unordered_map<EventType, std::vector<EventCallback>> m_Observers;

        std::vector<std::unique_ptr<Event>> m_Queue;
        std::vector<std::unique_ptr<Event>> m_FrontQueue;
    };
} // namespace Rose
