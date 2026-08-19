#include "SceneHierarchyPanelLayer.hpp"

#include "Thorn/ThornApp.hpp"
#include "Thorn/ViewportLayer.hpp"

#include <Rose/Rose.hpp>

namespace Rose {
    SceneHierarchyPanelLayer::SceneHierarchyPanelLayer() : m_Scene(ThornApp::GetViewportLayer()->GetScene()) {}

    void SceneHierarchyPanelLayer::OnImGuiRender()
    {
        entt::registry& rawScene = m_Scene.GetRegistry();

        static bool showCreateEntityModal = false;

        if (ImGui::Begin("SceneHierarchyPanel"))
        {
            for (auto entity : rawScene.view<NameComponent>())
            {
                ImGui::Text("%s", rawScene.get<NameComponent>(entity).Name.c_str());
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                {
                    m_SelectedEntity = entity;
                }
            }

            if (ImGui::BeginPopupContextWindow("HierarchyContextMenu"))
            {
                if (ImGui::MenuItem("Create entity"))
                {
                    showCreateEntityModal = true;
                }
                ImGui::EndPopup();
            }
        }
        ImGui::End();

        if (showCreateEntityModal)
        {
            ImGui::OpenPopup("CreateEntityModal");
        }

        if (ImGui::BeginPopupModal("CreateEntityModal", &showCreateEntityModal, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char buf[256] = "";

            if (ImGui::IsWindowAppearing())
                ImGui::SetKeyboardFocusHere();

            ImGui::InputText("Name", buf, 256);

            if (ImGui::Button("Create"))
            {
                entt::entity e = entt::null;

                if (buf[0] == '\0')
                    e = m_Scene.CreateEntity("No name");
                else
                    e = m_Scene.CreateEntity(std::string(buf));

                memset(buf, '\0', 256);
                showCreateEntityModal = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
            {
                showCreateEntityModal = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
} // namespace Rose
