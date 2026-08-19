#pragma once
#include "Rose/Core/Core.hpp"

#include <entt/entt.hpp>

#include "Rose/Scene/Camera.hpp"
#include "Rose/Scene/Component.hpp"

namespace Rose {
    class Scene
    {
    public:
        [[nodiscard]] entt::entity CreateEntity(std::string name = "No name")
        {
            entt::entity e = m_Registry.create();
            m_Registry.emplace<NameComponent>(e, std::move(name));
            return e;
        }

        void DestroyEntity(entt::entity e) { m_Registry.destroy(e); }
        bool Exists(entt::entity e) const { return m_Registry.valid(e); }

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
        void RemoveComponent(entt::entity e)
        {
            ASSERT(HasComponent<T>(e), "Entity does NOT have component");
            m_Registry.remove<T>(e);
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
            return m_Registry.all_of<T>(e);
        }

        entt::registry& GetRegistry() { return m_Registry; }

        void Render();

    private:
        entt::registry m_Registry;
    };
} // namespace Rose
