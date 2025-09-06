#pragma once
#include <Core/MathF.h>
#include <Rendering/Renderer.h>

namespace Reality {
    class Camera {
    public:
        Camera();
        ~Camera() = default;

        // Position and orientation
        void SetPosition(const MathF::Vector3f& position);
        void SetRotation(const MathF::Quaternion& rotation);
        void LookAt(const MathF::Vector3f& target, const MathF::Vector3f& up = MathF::Vector3f(0, 1, 0));

        // Projection settings
        void SetPerspective(float fovDegrees, float nearClip, float farClip);
        void SetOrthographic(float width, float height, float nearClip, float farClip);

        // Matrix access
        const MathF::Matrix4x4& GetViewMatrix() const;
        const MathF::Matrix4x4& GetProjectionMatrix() const;
        MathF::Matrix4x4 GetViewProjectionMatrix() const;
        MathF::Matrix4x4 GetAdjustedViewProjectionMatrix(const SwapChainDesc& scDesc) const;

        // Camera properties
        const MathF::Vector3f& GetPosition() const;
        const MathF::Quaternion& GetRotation() const;
        MathF::Vector3f GetForward() const;
        MathF::Vector3f GetRight() const;
        MathF::Vector3f GetUp() const;

        // Movement
        void Move(const MathF::Vector3f& delta);
        void MoveForward(float distance);
        void MoveRight(float distance);
        void MoveUp(float distance);

        // Rotation
        void Rotate(const MathF::Quaternion& rotation);
        void RotateYaw(float angleDegrees);
        void RotatePitch(float angleDegrees);
        void RotateRoll(float angleDegrees);

        // Rendering
        void Render() const;

    private:
        void UpdateViewMatrix();
        void UpdateProjectionMatrix();
        MathF::Matrix4x4 GetSurfacePretransformMatrix(const SwapChainDesc& scDesc) const;

        // Camera properties
        MathF::Vector3f m_Position;
        MathF::Quaternion m_Rotation;

        // Projection properties
        float m_FOV = 60.0f;
        float m_AspectRatio = 16.0f / 9.0f;
        float m_NearClip = 0.1f;
        float m_FarClip = 1000.0f;
        bool m_IsOrthographic = false;
        float m_OrthoWidth = 10.0f;
        float m_OrthoHeight = 10.0f;

        // Cached matrices
        MathF::Matrix4x4 m_ViewMatrix;
        MathF::Matrix4x4 m_ProjectionMatrix;

        // Dirty flags
        mutable bool m_ViewDirty = true;
        mutable bool m_ProjectionDirty = true;
    };
}