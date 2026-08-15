#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Rose {
    class Camera
    {
    public:
        float FieldOfView = 45.0f;
        float AspectRatio = 16.0f / 9.0f;
        float NearPlane = 0.1f;
        float FarPlane = 1000.0f;

        glm::vec3 Position{0.0f};
        glm::quat Rotation{1.0f, 0.0f, 0.0f, 0.0f};

    public:
        void SetPerspective(float fov, float aspect, float near, float far)
        {
            FieldOfView = fov;
            AspectRatio = aspect;
            NearPlane = near;
            FarPlane = far;
        }

        glm::mat4 GetViewMatrix() const
        {
            glm::vec3 forward = Rotation * glm::vec3(0.0f, 0.0f, -1.0f);
            glm::vec3 up = Rotation * glm::vec3(0.0f, 1.0f, 0.0f);

            glm::mat4 viewMatrix = glm::lookAt(Position, Position + forward, up);
            viewMatrix[1][1] *= -1;

            return viewMatrix;
        }

        glm::mat4 GetProjectionMatrix() const
        {
            return glm::perspective(glm::radians(FieldOfView), AspectRatio, NearPlane, FarPlane);
        }
    };
} // namespace Rose
