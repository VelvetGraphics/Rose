#pragma once

#include <Rose/Rose.hpp>

namespace Rose {
    class SceneHierarchyPanelLayer : public Layer
    {
    public:
        SceneHierarchyPanelLayer();

        void OnImGuiRender() override;

        entt::entity GetSelectedEntity() const { return m_SelectedEntity; }

    private:
        Scene& m_Scene;

        bool m_RightClicked = false;
        entt::entity m_SelectedEntity = entt::null;
    };
} // namespace Rose
