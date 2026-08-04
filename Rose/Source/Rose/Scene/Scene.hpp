#pragma once
#include "Rose/Core/Core.hpp"

#include <entt/entt.hpp>

namespace Rose {
    class Scene
    {
    public:
        entt::entity AddEntity() { return m_Registry.create(); }

        template<typename T>
        T& AddComponent(entt::entity e)
        {
            ASSERT(!HasComponent<T>(e), "Entity already has component");
            return m_Registry.emplace<T>(e);
        }

        template<typename T, typename... Args>
        T& AddComponent(entt::entity e, Args&&... args)
        {
            ASSERT(!HasComponent<T>(e), "Entity already has component");
            return m_Registry.emplace<T>(e, std::forward<Args>(args)...);
        }

        template<typename T>
        const T& GetComponent(entt::entity e) const
        {
            ASSERT(HasComponent<T>(e), "Entity does NOT have component");
            return m_Registry.get<T>(e);
        }

        template<typename T>
        T& GetComponent(entt::entity e)
        {
            ASSERT(HasComponent<T>(e), "Entity does NOT have component");
            return m_Registry.get<T>(e);
        }

        template<typename T>
        bool HasComponent(entt::entity e) const
        {
            return m_Registry.owned<T>(e);
        }

        entt::registry& GetRegistry() { return m_Registry; }

    private:
        entt::registry m_Registry;
    };
} // namespace Rose
