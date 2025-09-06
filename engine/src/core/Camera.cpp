#include "Camera.h"
#include "rendering/Renderer.h"
#include <cmath>

namespace Reality {
    Camera::Camera() {
        // Initialize camera at (0,0,5) looking at origin
        SetPosition(MathF::Vector3f(0, 0, 5));
        SetRotation(MathF::Quaternion::Identity());
        SetPerspective(60.0f, 0.1f, 100.0f);
        UpdateViewMatrix();
    }

    void Camera::SetPosition(const MathF::Vector3f& position) {
        m_Position = position;
        m_ViewDirty = true;
    }

    void Camera::SetRotation(const MathF::Quaternion& rotation) {
        m_Rotation = rotation;
        m_ViewDirty = true;
    }

    void Camera::LookAt(const MathF::Vector3f& target, const MathF::Vector3f& up) {
        // Calculate forward vector (from camera to target)
        const MathF::Vector3f forward = (target - m_Position).Normalized();
        const MathF::Vector3f right = up.Cross(forward).Normalized();
        const MathF::Vector3f upAdjusted = forward.Cross(right);

        // Create rotation matrix with proper coordinate system
        MathF::Matrix4x4 rotationMatrix;
        rotationMatrix(0,0) = right.x;     rotationMatrix(0,1) = right.y;     rotationMatrix(0,2) = right.z;     rotationMatrix(0,3) = 0;
        rotationMatrix(1,0) = upAdjusted.x; rotationMatrix(1,1) = upAdjusted.y; rotationMatrix(1,2) = upAdjusted.z; rotationMatrix(1,3) = 0;
        rotationMatrix(2,0) = forward.x;   rotationMatrix(2,1) = forward.y;   rotationMatrix(2,2) = forward.z;   rotationMatrix(2,3) = 0;
        rotationMatrix(3,0) = 0;           rotationMatrix(3,1) = 0;           rotationMatrix(3,2) = 0;           rotationMatrix(3,3) = 1;

        // Convert matrix to quaternion using MathF's FromMatrix method
        m_Rotation = MathF::Quaternion::FromMatrix(rotationMatrix);
        m_ViewDirty = true;
    }

    void Camera::SetPerspective(const float fovDegrees, const float nearPlane, const float farPlane) {
        const auto renderer = Renderer::GetInstance();
        const auto swapChain = renderer.GetSwapChain();
        const auto aspectRadio = static_cast<float>(swapChain->GetDesc().Width) / static_cast<float>(swapChain->GetDesc().Height);
        m_FOV = fovDegrees;
        m_AspectRatio = aspectRadio;
        m_NearClip = nearPlane;
        m_FarClip = farPlane;
        m_IsOrthographic = false;
        m_ProjectionDirty = true;
    }

    void Camera::SetOrthographic(const float width, const float height, const float nearPlane, const float farPlane) {
        m_OrthoWidth = width;
        m_OrthoHeight = height;
        m_NearClip = nearPlane;
        m_FarClip = farPlane;
        m_IsOrthographic = true;
        m_ProjectionDirty = true;
    }

    const MathF::Matrix4x4& Camera::GetViewMatrix() const {
        if (m_ViewDirty) {
            const_cast<Camera*>(this)->UpdateViewMatrix();
        }
        return m_ViewMatrix;
    }

    const MathF::Matrix4x4& Camera::GetProjectionMatrix() const {
        if (m_ProjectionDirty) {
            const_cast<Camera*>(this)->UpdateProjectionMatrix();
        }
        return m_ProjectionMatrix;
    }

    MathF::Matrix4x4 Camera::GetViewProjectionMatrix() const {
        return GetViewMatrix() * GetProjectionMatrix();
    }

    MathF::Matrix4x4 Camera::GetAdjustedViewProjectionMatrix(const SwapChainDesc& scDesc) const {
        const MathF::Matrix4x4 viewProj = GetViewMatrix() * GetProjectionMatrix();
        // Apply surface pre-transform
        const MathF::Matrix4x4 srfPreTransform = GetSurfacePretransformMatrix(scDesc);
        return viewProj * srfPreTransform;
    }

    const MathF::Vector3f& Camera::GetPosition() const {
        return m_Position;
    }

    const MathF::Quaternion& Camera::GetRotation() const {
        return m_Rotation;
    }

    MathF::Vector3f Camera::GetForward() const {
        // Convert quaternion to matrix and extract forward vector
        MathF::Matrix4x4 rotationMatrix = m_Rotation.ToMatrix();
        // In column-major matrix, forward is the third column
        // For Diligent's right-handed system, we need to negate Z
        return MathF::Vector3f(rotationMatrix(0,2), rotationMatrix(1,2), -rotationMatrix(2,2));
    }

    MathF::Vector3f Camera::GetRight() const {
        // Convert quaternion to matrix and extract right vector
        MathF::Matrix4x4 rotationMatrix = m_Rotation.ToMatrix();
        // In column-major matrix, right is the first column
        return MathF::Vector3f(rotationMatrix(0,0), rotationMatrix(1,0), rotationMatrix(2,0));
    }

    MathF::Vector3f Camera::GetUp() const {
        // Convert quaternion to matrix and extract up vector
        MathF::Matrix4x4 rotationMatrix = m_Rotation.ToMatrix();
        // In column-major matrix, up is the second column
        return MathF::Vector3f(rotationMatrix(0,1), rotationMatrix(1,1), rotationMatrix(2,1));
    }

    void Camera::Move(const MathF::Vector3f& delta) {
        SetPosition(m_Position + delta);
    }

    void Camera::MoveForward(const float distance) {
        Move(GetForward() * distance);
    }

    void Camera::MoveRight(const float distance) {
        Move(GetRight() * distance);
    }

    void Camera::MoveUp(const float distance) {
        Move(GetUp() * distance);
    }

    void Camera::Rotate(const MathF::Quaternion& rotation) {
        // Use MathF quaternion multiplication
        SetRotation(m_Rotation * rotation);
    }

    void Camera::RotateYaw(const float angleDegrees) {
        const float angleRadians = MathF::DegreesToRadians(angleDegrees);
        // Create rotation quaternion around Y axis using MathF
        const MathF::Quaternion rotation = MathF::Quaternion::FromAxisAngle(MathF::Vector3f(0, 1, 0), angleRadians);
        Rotate(rotation);
    }

    void Camera::RotatePitch(const float angleDegrees) {
        const float angleRadians = MathF::DegreesToRadians(angleDegrees);
        // Create rotation quaternion around X axis using MathF
        const MathF::Quaternion rotation = MathF::Quaternion::FromAxisAngle(MathF::Vector3f(1, 0, 0), angleRadians);
        Rotate(rotation);
    }

    void Camera::RotateRoll(const float angleDegrees) {
        const float angleRadians = MathF::DegreesToRadians(angleDegrees);
        // Create rotation quaternion around Z axis using MathF
        const MathF::Quaternion rotation = MathF::Quaternion::FromAxisAngle(MathF::Vector3f(0, 0, 1), angleRadians);
        Rotate(rotation);
    }

    void Camera::Render() const {
        // Apply the camera transformations.
        const auto& scDesc = Renderer::GetInstance().GetSwapChain()->GetDesc();
        const MathF::Matrix4x4 viewProj = GetAdjustedViewProjectionMatrix(scDesc);
        // Convert to Diligent type for rendering
        const auto diligentMatrix = static_cast<float4x4>(viewProj);
        Renderer::GetInstance().SetWorldProjectionMatrix(diligentMatrix);
    }

    void Camera::UpdateViewMatrix() {
        // Create view matrix from position and rotation
        // For Diligent's right-handed coordinate system, we need to:
        // 1. Translate by negative position
        // 2. Apply inverse rotation
        // In column-major matrices, multiplication order is reversed
        const MathF::Matrix4x4 translation = MathF::Matrix4x4::Translation(-m_Position);
        const MathF::Quaternion conjugate = m_Rotation.Conjugate();
        const MathF::Matrix4x4 rotationMatrix = conjugate.ToMatrix();

        // For column-major matrices: translation * rotation
        // This applies rotation first, then translation
        m_ViewMatrix = translation * rotationMatrix;
        m_ViewDirty = false;
    }

    void Camera::UpdateProjectionMatrix() {
        if (m_IsOrthographic) {
            // Orthographic projection
            const float left = -m_OrthoWidth / 2.0f;
            const float right = m_OrthoWidth / 2.0f;
            const float bottom = -m_OrthoHeight / 2.0f;
            const float top = m_OrthoHeight / 2.0f;
            m_ProjectionMatrix = MathF::Matrix4x4::Ortho(left, right, bottom, top, m_NearClip, m_FarClip, true);
        } else {
            // Perspective projection for right-handed coordinate system
            const float fovRad = MathF::DegreesToRadians(m_FOV);
            m_ProjectionMatrix = MathF::Matrix4x4::Perspective(fovRad, m_AspectRatio, m_NearClip, m_FarClip);
        }
        m_ProjectionDirty = false;
    }

    MathF::Matrix4x4 Camera::GetSurfacePretransformMatrix(const SwapChainDesc& scDesc) const {
        const MathF::Vector3f viewAxis = GetForward();
        switch (scDesc.PreTransform) {
            case SURFACE_TRANSFORM_ROTATE_90:
                return MathF::Matrix4x4::RotationArbitrary(viewAxis, -PI_F / 2.0f);
            case SURFACE_TRANSFORM_ROTATE_180:
                return MathF::Matrix4x4::RotationArbitrary(viewAxis, -PI_F);
            case SURFACE_TRANSFORM_ROTATE_270:
                return MathF::Matrix4x4::RotationArbitrary(viewAxis, -PI_F * 3.0f / 2.0f);
            case SURFACE_TRANSFORM_OPTIMAL:
            case SURFACE_TRANSFORM_IDENTITY:
            default:
                return MathF::Matrix4x4::Identity();
        }
    }
}