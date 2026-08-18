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

        float Yaw = 0.0f;
        float Pitch = 0.0f;

        glm::vec3 Position{0.0f};

    public:
        Camera();
        Camera(float fov, float aspect, float near, float far);

        ~Camera() = default;

        void SetPerspective(float fov, float aspect, float near, float far)
        {
            FieldOfView = fov;
            AspectRatio = aspect;
            NearPlane = near;
            FarPlane = far;
        }

        [[nodiscard]] glm::mat4 GetViewMatrix() const;
        [[nodiscard]] glm::mat4 GetProjectionMatrix() const;

        [[nodiscard]] glm::vec3 GetForward() const { return glm::normalize(Rotation() * glm::vec3(0.0f, 0.0f, -1.0f)); }
        [[nodiscard]] glm::vec3 GetRight() const { return normalize(Rotation() * glm::vec3(1.0f, 0.0f, 0.0f)); }
        [[nodiscard]] glm::vec3 GetUp() const { return glm::normalize(Rotation() * glm::vec3(0.0f, 1.0f, 0.0f)); }

    private:
        [[nodiscard]] glm::quat Rotation() const
        {
            glm::quat yaw = glm::angleAxis(glm::radians(Yaw), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat pitch = glm::angleAxis(glm::radians(Pitch), glm::vec3(1.0f, 0.0f, 0.0f));

            return glm::normalize(yaw * pitch);
        }
    };
} // namespace Rose
