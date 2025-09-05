#pragma once
#include <DiligentCore/Common/interface/BasicMath.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>

namespace Reality {
    class Camera {
    public:
        Camera();
        ~Camera() = default;

        // Position and orientation
        void SetPosition(const Diligent::float3& position);
        void SetRotation(const Diligent::Quaternion<float>& rotation);
        void LookAt(const Diligent::float3& target, const Diligent::float3& up = Diligent::float3(0, 1, 0));

        // Projection settings
        void SetPerspective(float fovDegrees, float aspectRatio, float nearClip, float farClip);
        void SetOrthographic(float width, float height, float nearClip, float farClip);

        // Matrix access
        const Diligent::float4x4& GetViewMatrix() const;
        const Diligent::float4x4& GetProjectionMatrix() const;
        Diligent::float4x4 GetViewProjectionMatrix() const;
        Diligent::float4x4 GetAdjustedViewProjectionMatrix(const Diligent::SwapChainDesc& scDesc) const;

        // Camera properties
        const Diligent::float3& GetPosition() const;
        const Diligent::Quaternion<float>& GetRotation() const;
        Diligent::float3 GetForward() const;
        Diligent::float3 GetRight() const;
        Diligent::float3 GetUp() const;

        // Movement
        void Move(const Diligent::float3& delta);
        void MoveForward(float distance);
        void MoveRight(float distance);
        void MoveUp(float distance);

        // Rotation
        void Rotate(const Diligent::Quaternion<float>& rotation);
        void RotateYaw(float angleDegrees);
        void RotatePitch(float angleDegrees);
        void RotateRoll(float angleDegrees);

    private:
        void UpdateViewMatrix();
        void UpdateProjectionMatrix();
        Diligent::float4x4 GetSurfacePretransformMatrix(const Diligent::SwapChainDesc& scDesc) const;

        // Camera properties
        Diligent::float3 m_Position;
        Diligent::Quaternion<float> m_Rotation;

        // Projection properties
        float m_FOV = 60.0f;
        float m_AspectRatio = 16.0f / 9.0f;
        float m_NearClip = 0.1f;
        float m_FarClip = 1000.0f;
        bool m_IsOrthographic = false;
        float m_OrthoWidth = 10.0f;
        float m_OrthoHeight = 10.0f;

        // Cached matrices
        Diligent::float4x4 m_ViewMatrix;
        Diligent::float4x4 m_ProjectionMatrix;

        // Dirty flags
        mutable bool m_ViewDirty = true;
        mutable bool m_ProjectionDirty = true;
    };
}