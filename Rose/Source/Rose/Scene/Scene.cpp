#include "Scene.hpp"

namespace Rose {
    void Scene::Render()
    {
        for (auto entity : m_Registry.view<Renderer3DComponent>())
        {
            const auto& component = m_Registry.get<Renderer3DComponent>(entity);
            component.Shader->Bind();
            component.Mesh->Bind();
            component.Mesh->Draw();
        }
    }
} // namespace Rose
