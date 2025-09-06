#include "TransformComponent.h"
#include "BaseGameObject.h"
#include <algorithm>
#include "Log.h"

namespace Reality {
    TransformComponent::TransformComponent()
        : m_localPosition(MathF::Vector3f(0, 0, 0)),
          m_localRotation(MathF::Quaternion::Identity()),
          m_localScale(MathF::Vector3f(1, 1, 1)),
          m_scale(MathF::Vector3f(1, 1, 1)),
          m_position(MathF::Vector3f(0, 0, 0)),
          m_rotation(MathF::Quaternion::Identity()),
          m_transformDirty(true), m_parent(nullptr) {
    }

    void TransformComponent::Start() {
        BaseComponent::Start();
        UpdateTransform();
    }

    void TransformComponent::Update(const float deltaTime) {
        BaseComponent::Update(deltaTime);
        const auto name = GetGameObject()->GetName();
        RLOG_INFO("[%s] - Transform Update",name.c_str());
        if (m_transformDirty) {
            UpdateTransform();
        }
    }

    void TransformComponent::SetPosition(const MathF::Vector3f& position) {
        if (m_parent) {
            SetLocalPosition(m_parent->InverseTransformPoint(position));
        } else {
            SetLocalPosition(position);
        }
    }

    void TransformComponent::SetRotation(const MathF::Quaternion& rotation) {
        if (m_parent) {
            SetLocalRotation(m_parent->GetRotation().Conjugate() * rotation);
        } else {
            SetLocalRotation(rotation);
        }
    }

    void TransformComponent::SetScale(const MathF::Vector3f& scale) {
        if (m_parent) {
            const MathF::Vector3f parentScale = m_parent->GetScale();
            SetLocalScale(MathF::Vector3f(
                scale.x / parentScale.x,
                scale.y / parentScale.y,
                scale.z / parentScale.z
            ));
        } else {
            SetLocalScale(scale);
        }
    }

    void TransformComponent::SetLocalPosition(const MathF::Vector3f& position) {
        m_localPosition = position;
        MarkTransformDirty();
    }

    void TransformComponent::SetLocalRotation(const MathF::Quaternion& rotation) {
        m_localRotation = rotation;
        MarkTransformDirty();
    }

    void TransformComponent::SetLocalScale(const MathF::Vector3f& scale) {
        m_localScale = scale;
        MarkTransformDirty();
    }

    MathF::Vector3f TransformComponent::GetPosition() const {
        if (m_transformDirty) UpdateTransform();
        return m_position;
    }

    MathF::Quaternion TransformComponent::GetRotation() const {
        if (m_transformDirty) UpdateTransform();
        return m_rotation;
    }

    MathF::Vector3f TransformComponent::GetScale() const {
        if (m_transformDirty) UpdateTransform();
        return m_scale;
    }

    MathF::Matrix4x4 TransformComponent::GetLocalToWorldMatrix() const {
        if (m_transformDirty) UpdateTransform();
        return m_localToWorldMatrix;
    }

    MathF::Matrix4x4 TransformComponent::GetWorldToLocalMatrix() const {
        if (m_transformDirty) UpdateTransform();
        return m_localToWorldMatrix.Inverse();
    }

    MathF::Vector3f TransformComponent::GetForward() const {
        if (m_transformDirty) UpdateTransform();
        return m_rotation * MathF::Vector3f(0, 0, 1);
    }

    MathF::Vector3f TransformComponent::GetRight() const {
        if (m_transformDirty) UpdateTransform();
        return m_rotation * MathF::Vector3f(1, 0, 0);
    }

    MathF::Vector3f TransformComponent::GetUp() const {
        if (m_transformDirty) UpdateTransform();
        return m_rotation * MathF::Vector3f(0, 1, 0);
    }

    MathF::Vector3f TransformComponent::TransformPoint(const MathF::Vector3f& point) const {
        return static_cast<MathF::Vector3f>(GetLocalToWorldMatrix() * MathF::Vector4f(point, 1.0f));
    }

    MathF::Vector3f TransformComponent::InverseTransformPoint(const MathF::Vector3f& point) const {
        return static_cast<MathF::Vector3f>(GetWorldToLocalMatrix() * MathF::Vector4f(point, 1.0f));
    }

    MathF::Vector3f TransformComponent::TransformDirection(const MathF::Vector3f& direction) const {
        return static_cast<MathF::Vector3f>(GetLocalToWorldMatrix() * MathF::Vector4f(direction, 0.0f));
    }

    MathF::Vector3f TransformComponent::InverseTransformDirection(const MathF::Vector3f& direction) const {
        return static_cast<MathF::Vector3f>(GetWorldToLocalMatrix() * MathF::Vector4f(direction, 0.0f));
    }

    void TransformComponent::SetParent(TransformComponent* parent) {
        if (m_parent == parent) return;

        // Remove from current parent
        if (m_parent) {
            m_parent->RemoveChild(this);
        }

        // Set new parent
        m_parent = parent;

        // Add to new parent
        if (m_parent) {
            m_parent->AddChild(this);
        }

        MarkTransformDirty();
    }

    void TransformComponent::AddChild(TransformComponent* child) {
        if (child && std::ranges::find(m_children, child) == m_children.end()) {
            m_children.push_back(child);
        }
    }

    void TransformComponent::RemoveChild(TransformComponent* child) {
        if (const auto it = std::ranges::find(m_children, child); it != m_children.end()) {
            m_children.erase(it);
        }
    }

    void TransformComponent::UpdateTransform() const {
        // Calculate local-to-world matrix
        const MathF::Matrix4x4 translation = MathF::Matrix4x4::Translation(m_localPosition);
        const MathF::Matrix4x4 rotation = m_localRotation.ToMatrix();
        const MathF::Matrix4x4 scale = MathF::Matrix4x4::Scale(m_localScale);

        const MathF::Matrix4x4 localMatrix = translation * rotation * scale;

        if (m_parent) {
            m_localToWorldMatrix = m_parent->GetLocalToWorldMatrix() * localMatrix;
            m_rotation = m_parent->GetRotation() * m_localRotation;
            m_scale = MathF::Vector3f(
                m_localScale.x * m_parent->GetScale().x,
                m_localScale.y * m_parent->GetScale().y,
                m_localScale.z * m_parent->GetScale().z
            );
        } else {
            m_localToWorldMatrix = localMatrix;
            m_rotation = m_localRotation;
            m_scale = m_localScale;
        }

        // Extract position from matrix
        m_position = MathF::Vector3f(
            m_localToWorldMatrix(0, 3),
            m_localToWorldMatrix(1, 3),
            m_localToWorldMatrix(2, 3)
        );

        m_transformDirty = false;
    }

    void TransformComponent::MarkTransformDirty() const {
        m_transformDirty = true;

        // Mark all children as dirty
        for (const TransformComponent* child : m_children) {
            child->MarkTransformDirty();
        }
    }
}
