#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Rose/Graphics/Mesh.hpp"
#include "Rose/Graphics/Shader.hpp"

namespace Rose {
    struct NameComponent
    {
        std::string Name;
    };

    struct TransformComponent
    {
        glm::vec3 position = glm::vec3(0.0f);
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
        glm::vec3 scale = glm::vec3(1.0f);

        [[nodiscard]] glm::mat4 Transform() const
        {
            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
            glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
            glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

            return translationMatrix * rotationMatrix * scaleMatrix;
        }
    };

    struct Renderer3DComponent
    {
        Ref<Mesh> Mesh;
        Ref<Shader> Shader;
    };
} // namespace Rose
