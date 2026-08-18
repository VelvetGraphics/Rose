#include "Camera.hpp"

namespace Rose {
    glm::mat4 Camera::GetViewMatrix() const
    {
        glm::quat rotation = Rotation();

        glm::vec3 forward = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 up = rotation * glm::vec3(0.0f, 1.0f, 0.0f);

        glm::mat4 viewMatrix = glm::lookAt(Position, Position + forward, up);

        return viewMatrix;
    }

    glm::mat4 Camera::GetProjectionMatrix() const
    {
        glm::mat4 projectionMatrix = glm::perspectiveRH_ZO(glm::radians(FieldOfView), AspectRatio, NearPlane, FarPlane);
        projectionMatrix[1][1] *= -1;
        return projectionMatrix;
    }
} // namespace Rose
