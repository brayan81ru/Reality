#include "Camera.h"

#include "rendering/Renderer.h"

namespace Reality {
    // Helper function to convert quaternion to matrix
    float4x4 QuaternionToMatrix(const Quaternion<float>& q) {
        float4x4 result;
        float xx = q.q.x * q.q.x;
        float xy = q.q.x * q.q.y;
        float xz = q.q.x * q.q.z;
        float xw = q.q.x * q.q.w;
        float yy = q.q.y * q.q.y;
        float yz = q.q.y * q.q.z;
        float yw = q.q.y * q.q.w;
        float zz = q.q.z * q.q.z;
        float zw = q.q.z * q.q.w;

        result._11 = 1 - 2 * (yy + zz);
        result._12 = 2 * (xy - zw);
        result._13 = 2 * (xz + yw);
        result._14 = 0;

        result._21 = 2 * (xy + zw);
        result._22 = 1 - 2 * (xx + zz);
        result._23 = 2 * (yz - xw);
        result._24 = 0;

        result._31 = 2 * (xz - yw);
        result._32 = 2 * (yz + xw);
        result._33 = 1 - 2 * (xx + yy);
        result._34 = 0;

        result._41 = 0;
        result._42 = 0;
        result._43 = 0;
        result._44 = 1;

        return result;
    }

    // Helper function to convert matrix to quaternion
    Quaternion<float> MatrixToQuaternion(const float4x4& m) {
        Quaternion<float> q;
        float trace = m._11 + m._22 + m._33;

        if (trace > 0) {
            float s = 0.5f / sqrtf(trace + 1.0f);
            q.q.w = 0.25f / s;
            q.q.x = (m._32 - m._23) * s;
            q.q.y = (m._13 - m._31) * s;
            q.q.z = (m._21 - m._12) * s;
        } else if (m._11 > m._22 && m._11 > m._33) {
            float s = 2.0f * sqrtf(1.0f + m._11 - m._22 - m._33);
            q.q.w = (m._32 - m._23) / s;
            q.q.x = 0.25f * s;
            q.q.y = (m._12 + m._21) / s;
            q.q.z = (m._13 + m._31) / s;
        } else if (m._22 > m._33) {
            float s = 2.0f * sqrtf(1.0f + m._22 - m._11 - m._33);
            q.q.w = (m._13 - m._31) / s;
            q.q.x = (m._12 + m._21) / s;
            q.q.y = 0.25f * s;
            q.q.z = (m._23 + m._32) / s;
        } else {
            float s = 2.0f * sqrtf(1.0f + m._33 - m._11 - m._22);
            q.q.w = (m._21 - m._12) / s;
            q.q.x = (m._13 + m._31) / s;
            q.q.y = (m._23 + m._32) / s;
            q.q.z = 0.25f * s;
        }

        return q;
    }

    // Helper function to compute quaternion conjugate
    Quaternion<float> QuaternionConjugate(const Quaternion<float>& q) {
        Quaternion<float> result;
        result.q.x = -q.q.x;
        result.q.y = -q.q.y;
        result.q.z = -q.q.z;
        result.q.w = q.q.w;
        return result;
    }

    Camera::Camera() {
        SetPosition(float3(0, 0, 5));
        // Create identity quaternion manually
        m_Rotation.q.x = 0;
        m_Rotation.q.y = 0;
        m_Rotation.q.z = 0;
        m_Rotation.q.w = 1;
        SetPerspective(60.0f, 0.1f, 1000.0f);
        UpdateViewMatrix();
    }

    void Camera::SetPosition(const float3& position) {
        m_Position = position;
        m_ViewDirty = true;
    }

    void Camera::SetRotation(const Quaternion<float>& rotation) {
        m_Rotation = rotation;
        m_ViewDirty = true;
    }

    void Camera::LookAt(const float3& target, const float3& up) {
        float3 forward = normalize(target - m_Position);
        float3 right = normalize(cross(up, forward));
        float3 upAdjusted = cross(forward, right);

        // Create rotation matrix
        float4x4 rotationMatrix;
        rotationMatrix._11 = right.x; rotationMatrix._12 = right.y; rotationMatrix._13 = right.z; rotationMatrix._14 = 0;
        rotationMatrix._21 = upAdjusted.x; rotationMatrix._22 = upAdjusted.y; rotationMatrix._23 = upAdjusted.z; rotationMatrix._24 = 0;
        rotationMatrix._31 = forward.x; rotationMatrix._32 = forward.y; rotationMatrix._33 = forward.z; rotationMatrix._34 = 0;
        rotationMatrix._41 = 0; rotationMatrix._42 = 0; rotationMatrix._43 = 0; rotationMatrix._44 = 1;

        // Convert matrix to quaternion using helper function
        m_Rotation = MatrixToQuaternion(rotationMatrix);
        m_ViewDirty = true;
    }

    void Camera::SetPerspective(const float fovDegrees, const float nearClip, const float farClip) {
        const auto renderer = Renderer::GetInstance();
        const auto swapChain = renderer.GetSwapChain();
        const auto aspectRadio =static_cast<float>(swapChain->GetDesc().Width)/static_cast<float>(swapChain->GetDesc().Height);
        m_FOV = fovDegrees;
        m_AspectRatio = aspectRadio;
        m_NearClip = nearClip;
        m_FarClip = farClip;
        m_IsOrthographic = false;
        m_ProjectionDirty = true;
    }

    void Camera::SetOrthographic(float width, float height, float nearClip, float farClip) {
        m_OrthoWidth = width;
        m_OrthoHeight = height;
        m_NearClip = nearClip;
        m_FarClip = farClip;
        m_IsOrthographic = true;
        m_ProjectionDirty = true;
    }

    const float4x4& Camera::GetViewMatrix() const {
        if (m_ViewDirty) {
            const_cast<Camera*>(this)->UpdateViewMatrix();
        }
        return m_ViewMatrix;
    }

    const float4x4& Camera::GetProjectionMatrix() const {
        if (m_ProjectionDirty) {
            const_cast<Camera*>(this)->UpdateProjectionMatrix();
        }
        return m_ProjectionMatrix;
    }

    float4x4 Camera::GetViewProjectionMatrix() const {
        return GetViewMatrix() * GetProjectionMatrix();
    }

    float4x4 Camera::GetAdjustedViewProjectionMatrix(const SwapChainDesc& scDesc) const {
        float4x4 viewProj = GetViewMatrix() * GetProjectionMatrix();

        // Apply surface pre-transform
        float4x4 srfPreTransform = GetSurfacePretransformMatrix(scDesc);
        return viewProj * srfPreTransform;
    }

    const float3& Camera::GetPosition() const {
        return m_Position;
    }

    const Quaternion<float>& Camera::GetRotation() const {
        return m_Rotation;
    }

    float3 Camera::GetForward() const {
        // Convert quaternion to matrix using helper function and extract forward vector
        float4x4 rotationMatrix = QuaternionToMatrix(m_Rotation);
        return {rotationMatrix._31, rotationMatrix._32, rotationMatrix._33};
    }

    float3 Camera::GetRight() const {
        // Convert quaternion to matrix using helper function and extract right vector
        float4x4 rotationMatrix = QuaternionToMatrix(m_Rotation);
        return {rotationMatrix._11, rotationMatrix._12, rotationMatrix._13};
    }

    float3 Camera::GetUp() const {
        // Convert quaternion to matrix using helper function and extract up vector
        float4x4 rotationMatrix = QuaternionToMatrix(m_Rotation);
        return {rotationMatrix._21, rotationMatrix._22, rotationMatrix._23};
    }

    void Camera::Move(const float3& delta) {
        SetPosition(m_Position + delta);
    }

    void Camera::MoveForward(float distance) {
        Move(GetForward() * distance);
    }

    void Camera::MoveRight(float distance) {
        Move(GetRight() * distance);
    }

    void Camera::MoveUp(float distance) {
        Move(GetUp() * distance);
    }

    void Camera::Rotate(const Quaternion<float>& rotation) {
        // Quaternion multiplication
        Quaternion<float> result;
        result.q.x = m_Rotation.q.w * rotation.q.x + m_Rotation.q.x * rotation.q.w + m_Rotation.q.y * rotation.q.z - m_Rotation.q.z * rotation.q.y;
        result.q.y = m_Rotation.q.w * rotation.q.y - m_Rotation.q.x * rotation.q.z + m_Rotation.q.y * rotation.q.w + m_Rotation.q.z * rotation.q.x;
        result.q.z = m_Rotation.q.w * rotation.q.z + m_Rotation.q.x * rotation.q.y - m_Rotation.q.y * rotation.q.x + m_Rotation.q.z * rotation.q.w;
        result.q.w = m_Rotation.q.w * rotation.q.w - m_Rotation.q.x * rotation.q.x - m_Rotation.q.y * rotation.q.y - m_Rotation.q.z * rotation.q.z;

        SetRotation(result);
    }

    void Camera::RotateYaw(float angleDegrees) {
        float angleRadians = angleDegrees * PI_F / 180.0f;
        float halfAngle = angleRadians * 0.5f;

        // Create rotation quaternion around Y axis
        Quaternion<float> rotation;
        rotation.q.x = 0;
        rotation.q.y = sinf(halfAngle);
        rotation.q.z = 0;
        rotation.q.w = cosf(halfAngle);

        Rotate(rotation);
    }

    void Camera::RotatePitch(float angleDegrees) {
        float angleRadians = angleDegrees * PI_F / 180.0f;
        float halfAngle = angleRadians * 0.5f;

        // Create rotation quaternion around X axis
        Quaternion<float> rotation;
        rotation.q.x = sinf(halfAngle);
        rotation.q.y = 0;
        rotation.q.z = 0;
        rotation.q.w = cosf(halfAngle);

        Rotate(rotation);
    }

    void Camera::RotateRoll(float angleDegrees) {
        float angleRadians = angleDegrees * PI_F / 180.0f;
        float halfAngle = angleRadians * 0.5f;

        // Create rotation quaternion around Z axis
        Quaternion<float> rotation;
        rotation.q.x = 0;
        rotation.q.y = 0;
        rotation.q.z = sinf(halfAngle);
        rotation.q.w = cosf(halfAngle);

        Rotate(rotation);
    }

    void Camera::Render() const {
        // Apply the camera transformations.

        const auto& scDesc = Renderer::GetInstance().GetSwapChain()->GetDesc();
        const float4x4 viewProj = GetAdjustedViewProjectionMatrix(scDesc);
        Renderer::GetInstance().SetWorldProjectionMatrix(viewProj);
    }

    void Camera::UpdateViewMatrix() {
        // Create view matrix from position and rotation
        float4x4 translation = float4x4::Translation(-m_Position);

        // Create rotation matrix from quaternion conjugate using helper functions
        Quaternion<float> conjugate = QuaternionConjugate(m_Rotation);
        float4x4 rotationMatrix = QuaternionToMatrix(conjugate);

        m_ViewMatrix = rotationMatrix * translation;
        m_ViewDirty = false;
    }

    void Camera::UpdateProjectionMatrix() {
        if (m_IsOrthographic) {
            // Orthographic projection
            float left = -m_OrthoWidth / 2.0f;
            float right = m_OrthoWidth / 2.0f;
            float bottom = -m_OrthoHeight / 2.0f;
            float top = m_OrthoHeight / 2.0f;

            m_ProjectionMatrix = float4x4::Ortho(1, 1, m_NearClip, m_FarClip, true);
        } else {
            // Perspective projection
            const float fovRad = m_FOV * PI_F / 180.0f;
            m_ProjectionMatrix = float4x4::Projection(fovRad, m_AspectRatio, m_NearClip, m_FarClip, true);
        }
        m_ProjectionDirty = false;
    }

    float4x4 Camera::GetSurfacePretransformMatrix(const SwapChainDesc& scDesc) const {
        float3 viewAxis = GetForward();

        switch (scDesc.PreTransform) {
            case SURFACE_TRANSFORM_ROTATE_90:
                return float4x4::RotationArbitrary(viewAxis, -PI_F / 2.0f);
            case SURFACE_TRANSFORM_ROTATE_180:
                return float4x4::RotationArbitrary(viewAxis, -PI_F);
            case SURFACE_TRANSFORM_ROTATE_270:
                return float4x4::RotationArbitrary(viewAxis, -PI_F * 3.0f / 2.0f);
            case SURFACE_TRANSFORM_OPTIMAL:
            case SURFACE_TRANSFORM_IDENTITY:
            default:
                return float4x4::Identity();
        }
    }
}
